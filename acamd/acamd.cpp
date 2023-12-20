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
  std::string logpath; 
  long ret=NO_ERROR;
  std::string daemon_in;     // daemon setting read from config file
  std::string daemon_stdout; // where daemon sends stdout
  std::string daemon_stderr; // where daemon sends stderr
  bool start_daemon = false; // don't start as daemon unless specifically requested

  Py_BEGIN_ALLOW_THREADS

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
    if ( configkey == "TM_ZONE") zone = configval;
    if ( configkey == "DAEMON")  daemon_in = configval;
    if ( configkey == "STDOUT")  daemon_stdout = configval;
    if ( configkey == "STDERR")  daemon_stderr = configval;
  }
  if (logpath.empty()) {
    logwrite(function, "ERROR: LOGPATH not specified in configuration file");
    acamd.exit_cleanly();
  }

  if ( !daemon_in.empty() && daemon_in == "yes" ) start_daemon = true;
  else
  if ( !daemon_in.empty() && daemon_in == "no"  ) start_daemon = false;
  else {
    message.str(""); message << "ERROR: unrecognized argument DAEMON=" << daemon_in << ", expected { yes | no }";
    logwrite( function, message.str() );
    acamd.exit_cleanly();
  }

  // check for "-d" command line option last so that the command line
  // can override the config file to start as daemon
  //
  if ( cmdOptionExists( argv, argv+argc, "-d" ) ) {
    start_daemon = true;
  }

  // daemonize, but don't close all file descriptors, required for the Andor camera
  //
  if ( start_daemon ) {
    logwrite( function, "starting daemon" );
    Daemon::daemonize( Acam::DAEMON_NAME, "/tmp", daemon_stdout, daemon_stderr, "", false );
  }

  if ( ( init_log( logpath, Acam::DAEMON_NAME ) != 0 ) ) {           // initialize the logging system
    logwrite(function, "ERROR: unable to initialize logging system");
    acamd.exit_cleanly();
  }

  message << "this version built " << BUILD_DATE << " " << BUILD_TIME;
  logwrite(function, message.str());

  message.str(""); message << acamd.config.n_entries << " lines read from " << acamd.config.filename;
  logwrite(function, message.str());

  if (ret==NO_ERROR) ret=acamd.configure_acamd();          // get needed values out of read-in configuration file for the daemon

  if (ret==NO_ERROR) ret=acamd.interface.configure_interface( acamd.config );

  if (ret != NO_ERROR) {
    logwrite(function, "ERROR: unable to configure system");
    acamd.exit_cleanly();
  }

  if (acamd.nbport == -1 || acamd.blkport == -1) {
    logwrite(function, "ERROR: acamd ports not configured");
    acamd.exit_cleanly();
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

  Network::TcpSocket s(acamd.blkport, true, -1, 0);  // instantiate TcpSocket object with blocking port
  s.Listen();                                        // create a listening socket
  socklist.push_back(s);                             // add it to the socklist vector
  std::thread( std::ref(Acam::Server::block_main),
               std::ref(acamd),
               std::ref(socklist[0]) ).detach();     // spawn a thread to handle requests on this socket

  // pre-thread N_THREADS-1 detached threads to handle requests on the non-blocking port
  // thread #0 is reserved for the blocking port (above)
  //
  for (int i=1; i<N_THREADS; i++) {                  // create N_THREADS-1 non-blocking socket objects
    if (i==1) {                                      // first one only
      Network::TcpSocket s(acamd.nbport, false, CONN_TIMEOUT, i);   // TcpSocket object, non-blocking port, CONN_TIMEOUT timeout
      s.Listen();                                    // create a listening socket
      socklist.push_back(s);
    }
    else {                                           // subsequent socket objects are copies of the first
      Network::TcpSocket s = socklist[1];            // copy the first one, which has a valid listening socket
      s.id = i;
      socklist.push_back(s);
    }
    std::thread( std::ref(Acam::Server::thread_main),
                 std::ref(acamd),
                 socklist[i] ).detach();  // spawn a thread to handle each non-blocking socket request
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

  for (;;) pause();                                  // main thread suspends
  Py_END_ALLOW_THREADS
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
  std::string function = "Acam::signal_handler";
  std::stringstream message;

  switch (signo) {
    case SIGTERM:
    case SIGINT:
      logwrite(function, "received termination signal");
      message << "NOTICE:" << Acam::DAEMON_NAME << " exit";
      acamd.interface.async.enqueue( message.str() );
      acamd.exit_cleanly();                      // shutdown the daemon
      break;
    case SIGHUP:
      acamd.interface.configure_interface( acamd.config );
      break;
    case SIGPIPE:
      logwrite(function, "ignored SIGPIPE");
      break;
    default:
      message << "received unknown signal " << strsignal(signo);
      logwrite( function, message.str() );
      message.str(""); message << "NOTICE:" << Acam::DAEMON_NAME << " exit";
      acamd.interface.async.enqueue( message.str() );
      break;
  }
  return;
}
/***** signal_handler *********************************************************/

