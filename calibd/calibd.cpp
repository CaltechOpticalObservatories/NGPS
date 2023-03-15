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
  std::string logpath;
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
    if (calibd.config.param[entry] == "TM_ZONE") zone = calibd.config.arg[entry];
    if (calibd.config.param[entry] == "DAEMON")  daemon_in = calibd.config.arg[entry];
  }
  if (logpath.empty()) {
    logwrite(function, "ERROR: LOGPATH not specified in configuration file");
    calibd.exit_cleanly();
  }

  if ( !daemon_in.empty() && daemon_in == "yes" ) start_daemon = true;
  else
  if ( !daemon_in.empty() && daemon_in == "no"  ) start_daemon = false;
  else {
    message.str(""); message << "ERROR: unrecognized argument DAEMON=" << daemon_in << ", expected { yes | no }";
    logwrite( function, message.str() );
    calibd.exit_cleanly();
  }

  // check for "-d" command line option last so that the command line
  // can override the config file to start as daemon
  //
  if ( cmdOptionExists( argv, argv+argc, "-d" ) ) {
    start_daemon = true;
  }

  if ( start_daemon ) {
    logwrite( function, "starting daemon" );
    Daemon::daemonize( Calib::DAEMON_NAME, "/tmp", "", "", "" );
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
  s.Listen();                                        // create a listening socket
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
      s.Listen();                                    // create a listening socket
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


/***** signal_handler *********************************************************/
/**
 * @brief      handles ctrl-C
 * @param[in]  signo
 *
 */
void signal_handler(int signo) {
  std::string function = "Calib::signal_handler";
  std::stringstream message;

  switch (signo) {
    case SIGTERM:
    case SIGINT:
      logwrite(function, "received termination signal");
      message << "NOTICE:" << Calib::DAEMON_NAME << " exit";
      calibd.interface.async.enqueue( message.str() );
      calibd.exit_cleanly();                     // shutdown the daemon
      break;
    case SIGHUP:
      logwrite(function, "caught SIGHUP");
      break;
    case SIGPIPE:
      logwrite(function, "caught SIGPIPE");
      break;
    default:
      message << "received unknown signal " << strsignal(signo);
      logwrite( function, message.str() );
      message.str(""); message << "NOTICE:" << Calib::DAEMON_NAME << " exit";
      calibd.interface.async.enqueue( message.str() );
      calibd.exit_cleanly();                     // shutdown the daemon
      break;
  }
  return;
}
/***** signal_handler *********************************************************/
