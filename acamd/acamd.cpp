/**
 * @file    acamd.cpp
 * @brief   this is the main acam daemon
 * @details the ACAM daemon is for the Acquisition (and guide) CAMera system
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 * This file has the required global function main()
 *
 */

#include "acamd.h"

/***** main *******************************************************************/
/**
 * @brief      the main function
 * @param[in]  int argc
 * @param[in]  char** argv
 * @return     0
 *
 */
int main(int argc, char **argv) {
  std::string function = "Acam::main";
  std::stringstream message;
  bool start_daemon = true;

  // Allow running in the foreground
  //
  if ( cmdOptionExists( argv, argv+argc, "--foreground" ) ) {
    start_daemon = false;
  }

  // TODO make configurable
  //
  std::string daemon_stdout="/dev/null";                          // where daemon sends stdout
  std::string daemon_stderr="/tmp/"+Acam::DAEMON_NAME+".stderr";  // where daemon sends stderr

  // daemonize, but don't close all file descriptors, required for the Andor camera
  //
  if ( start_daemon ) {
    logwrite( function, "starting daemon" );
    Daemon::daemonize( Acam::DAEMON_NAME, "/tmp", daemon_stdout, daemon_stderr, "", false );
  }

  if ( getpid() != getppid() ) {
  message.str(""); message << "XPA_NSUSERS=" << std::getenv("XPA_NSUSERS") << " PYTHONPATH=" << std::getenv("PYTHONPATH");
  logwrite( function, message.str() );

  logwrite( function, "daemonized. child process running" );

  // Now the child process instantiates a Server object
  //
  Acam::Server acamd;

  // Python should not be started by the parent. Initialize the Python objects
  // only by the child process, after daemonizing.
  //
  if ( acamd.initialize_python_objects() != NO_ERROR ) {
    logwrite( function, "ERROR initializing Python objects" );  // TODO should I allow things to continue?
  }
  PyEval_SaveThread();

  std::string logpath;
  long ret=NO_ERROR;

  // check for "-f <filename>" command line option to specify config file
  //
  if ( cmdOptionExists( argv, argv+argc, "-f" ) ) {
    char* filename = getCmdOption( argv, argv+argc, "-f" );
    if ( filename ) {
      acamd.config.filename = std::string( filename );
    }
  }
  else

  // if no "-f <filename>" then as long as there's at least one arg,
  // assume that is the config file name.
  //
  if (argc>1) {
    acamd.config.filename = std::string( argv[1] );
  }
  else {
    logwrite(function, "ERROR: no configuration file specified");
    acamd.exit_cleanly();
  }

  if ( acamd.config.read_config(acamd.config) != NO_ERROR) {          // read configuration file specified on command line
    logwrite(function, "ERROR: unable to configure system");
    acamd.exit_cleanly();
  }

  for (int entry=0; entry < acamd.config.n_entries; entry++) {

    std::string configkey;          // the current configuration key read from file
    std::string configval;          // the current configuration value read from file

    try {
      configkey = acamd.config.param.at(entry);
      configval = acamd.config.arg.at(entry);
    }
    catch (std::out_of_range &) {   // should be impossible
      message.str(""); message << "ERROR: entry " << entry 
                               << " out of range in config parameters (" << acamd.config.n_entries << ")";
      logwrite( function, message.str() );
      return(ERROR);
    }

    if ( configkey == "LOGPATH") logpath = configval;

    if (acamd.config.param[entry] == "TM_ZONE") {
      if ( acamd.config.arg[entry] != "UTC" && acamd.config.arg[entry] != "local" ) {
        message.str(""); message << "ERROR invalid TM_ZONE=" << acamd.config.arg[entry] << ": expected UTC|local";
        logwrite( function, message.str() );
        acamd.exit_cleanly();
      }
      tmzone_cfg = acamd.config.arg[entry];
      message.str(""); message << "config:" << acamd.config.param[entry] << "=" << acamd.config.arg[entry];
      logwrite( function, message.str() );
      acamd.interface.async.enqueue( message.str() );
    }

  }

  if (logpath.empty()) {
    logwrite(function, "ERROR: LOGPATH not specified in configuration file");
    acamd.exit_cleanly();
  }

  if ( ( init_log( logpath, Acam::DAEMON_NAME ) != 0 ) ) {           // initialize the logging system
    logwrite(function, "ERROR: unable to initialize logging system");
    acamd.exit_cleanly();
  }

  message.str(""); message << "this version built " << BUILD_DATE << " " << BUILD_TIME;
  logwrite(function, message.str());

  message.str(""); message << acamd.config.n_entries << " lines read from " << acamd.config.filename;
  logwrite(function, message.str());

  if (ret==NO_ERROR) ret=acamd.configure_acamd();          // get needed values out of read-in configuration file for the daemon
  if (ret==NO_ERROR) ret=acamd.configure_telemetry();      // get needed values out of read-in configuration file for telemetry

  if (ret==NO_ERROR) ret=acamd.interface.configure_interface( acamd.config );

  if (ret != NO_ERROR) {
    logwrite(function, "ERROR: unable to configure system");
    acamd.exit_cleanly();
  }

  if (acamd.nbport == -1 || acamd.blkport == -1) {
    logwrite(function, "ERROR: acamd ports not configured");
    acamd.exit_cleanly();
  }

  // initialize the pub/sub handler, which
  // takes a list of subscription topics
  //
  if ( acamd.interface.init_pubsub( {"tcsd", "targetinfo", "slitd"} ) == ERROR ) {
    logwrite(function, "ERROR initializing publisher-subscriber handler");
    acamd.exit_cleanly();
  }

  // This will pre-thread N_THREADS threads.
  // There will be N_THREADS-1 non-blocking threads, then
  // loop forever on Accept to dynamically spawn a new thread for each
  // blocking port connection (released when client disconnects).
  //
  // Each thread gets a socket object. All of the socket objects are stored in a map container.
  //
  // TcpSocket objects are instantiated with (PORT#, BLOCKING_STATE, POLL_TIMEOUT_MSEC, THREAD_ID#)
  //

  // pre-thread N_THREADS-1 detached threads to handle requests on the non-blocking port
  // thread #0 is reserved for the blocking port (above)
  //

  // First create the threads and add them to the map container
  //
  Network::TcpSocket sock_main(acamd.nbport, false, CONN_TIMEOUT, 1);    // instantiate TcpSocket object, non-blocking port, CONN_TIMEOUT timeout
  if ( sock_main.Listen() < 0 ) {                                        // create a listening socket
    logwrite( function, "ERROR could not create listening socket" );
    acamd.exit_cleanly();
  }
  acamd.socklist[1] = std::make_shared<Network::TcpSocket>(sock_main);   // add it to the socklist map

  for (int i = 2; i < Acam::N_THREADS; i++) {
    acamd.socklist[i] = std::make_shared<Network::TcpSocket>(sock_main); // copy the first one, which has a valid listening socket
    acamd.socklist[i]->id = i;                                           // update the id of the copied socket
  }

  // Create the threads for the above sockets
  //
  for ( int i = 1; i < Acam::N_THREADS; i++) {
    std::thread( Acam::Server::thread_main,
                 std::ref(acamd),
                 acamd.socklist[i] ).detach();
  }

  // Instantiate a multicast UDP object and spawn a thread to send asynchronous messages
  //
  Network::UdpSocket async(acamd.asyncport, acamd.asyncgroup);
  std::thread( std::ref(Acam::Server::async_main),
               std::ref(acamd),
               async ).detach();

  // thread to start a new logbook each day
  //
  std::thread( std::ref(Acam::Server::new_log_day), logpath ).detach();

  // Dynamically create a new listening socket and thread to handle
  // each connection request on the blocking port.
  //
  Network::TcpSocket sock_block(acamd.blkport, true, -1, 0);             // instantiate TcpSocket object with blocking port
  if ( sock_block.Listen() < 0 ) {                                       // create a listening socket
    logwrite( function, "ERROR could not create listening socket" );
    acamd.exit_cleanly();
  }

  while (true) {
    auto newid = acamd.id_pool.get_next_number();  // get the next available number from the pool

    // Lock the mutex before creating and initializing the new socket
    //
    {
    std::lock_guard<std::mutex> lock(acamd.sock_block_mutex);
    acamd.socklist[newid] = std::make_shared<Network::TcpSocket>(sock_block);  // create a new socket
    acamd.socklist[newid]->id = newid;        // update the id of the copied socket
    acamd.socklist[newid]->Accept();          // accept connections on the new socket
    }

    // Create a new thread to handle the connection
    //
    std::thread(Acam::Server::block_main, std::ref(acamd), acamd.socklist[newid]).detach();
  }

  for (;;) pause();                                  // main thread suspends

  }
  return 0;
}
/***** main *******************************************************************/
