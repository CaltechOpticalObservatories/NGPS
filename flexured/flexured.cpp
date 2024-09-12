/**
 * @file    flexured.cpp
 * @brief   this is the main flexure daemon for communicating with the flexure motors
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#include "flexured.h"


/***** main *******************************************************************/
/**
 * @brief      the main function
 * @param[in]  argc  argument count
 * @param[in]  argv  character array that holds all arguments passed to the “main()” function through the command line
 * @return     0
 *
 */
int main(int argc, char **argv) {
  std::string function = "Flexure::main";
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
      flexured.config.filename = std::string( filename );
    }
  }
  else

  // if no "-f <filename>" then as long as there's at least one arg,
  // assume that is the config file name.
  //
  if (argc>1) {
    flexured.config.filename = std::string( argv[1] );
  }
  else {
    logwrite(function, "ERROR: no configuration file specified");
    flexured.exit_cleanly();
  }

  if ( flexured.config.read_config(flexured.config) != NO_ERROR) {    // read configuration file specified on command line
    logwrite(function, "ERROR: unable to configure system");
    flexured.exit_cleanly();
  }

  for (int entry=0; entry < flexured.config.n_entries; entry++) {
    if (flexured.config.param[entry] == "LOGPATH") logpath = flexured.config.arg[entry];
    if (flexured.config.param[entry] == "DAEMON")  daemon_in = flexured.config.arg[entry];

    if (flexured.config.param[entry] == "TM_ZONE") {
      if ( flexured.config.arg[entry] != "UTC" && flexured.config.arg[entry] != "local" ) {
        message.str(""); message << "ERROR invalid TM_ZONE=" << flexured.config.arg[entry] << ": expected UTC|local";
        logwrite( function, message.str() );
        flexured.exit_cleanly();
      }
      tmzone_cfg = flexured.config.arg[entry];
      message.str(""); message << "config:" << flexured.config.param[entry] << "=" << flexured.config.arg[entry];
      logwrite( function, message.str() );
    }

  }

  if (logpath.empty()) {
    logwrite(function, "ERROR: LOGPATH not specified in configuration file");
    flexured.exit_cleanly();
  }

  if ( !daemon_in.empty() && daemon_in == "yes" ) start_daemon = true;
  else
  if ( !daemon_in.empty() && daemon_in == "no"  ) start_daemon = false;
  else {
    message.str(""); message << "ERROR: unrecognized argument DAEMON=" << daemon_in << ", expected { yes | no }";
    logwrite( function, message.str() );
    flexured.exit_cleanly();
  }

  // check for "-d" command line option last so that the command line
  // can override the config file to start as daemon
  //
  if ( cmdOptionExists( argv, argv+argc, "-d" ) ) {
    start_daemon = true;
  }

  if ( start_daemon ) {
    logwrite( function, "starting daemon" );
    Daemon::daemonize( Flexure::DAEMON_NAME, "/tmp", "", "", "" );
  }

  if ( ( init_log( logpath, Flexure::DAEMON_NAME ) != 0 ) ) {        // initialize the logging system
    logwrite(function, "ERROR: unable to initialize logging system");
    flexured.exit_cleanly();
  }

  message << "this version built " << BUILD_DATE << " " << BUILD_TIME;
  logwrite(function, message.str());

  message.str(""); message << flexured.config.n_entries << " lines read from " << flexured.config.filename;
  logwrite(function, message.str());

  if (ret==NO_ERROR) ret=flexured.configure_flexured();    // get needed values out of read-in configuration file for the daemon

  if (ret != NO_ERROR) {
    logwrite(function, "ERROR: unable to configure system");
    flexured.exit_cleanly();
  }

  if (flexured.nbport == -1 || flexured.blkport == -1) {
    logwrite(function, "ERROR: flexured ports not configured");
    flexured.exit_cleanly();
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

  // pre-thread N_THREADS-1 detached threads to handle requests on the non-blocking port
  // thread #0 is reserved for the blocking port (above)
  //

  // First create the threads and add them to the map container
  //
  Network::TcpSocket sock_main(flexured.nbport, false, CONN_TIMEOUT, 1);     // instantiate TcpSocket object, non-blocking port, CONN_TIMEOUT timeout

  if ( sock_main.Listen() < 0 ) {                                            // create a listening socket
    logwrite( function, "ERROR could not create listening socket" );
    flexured.exit_cleanly();
  }

  flexured.socklist[1] = std::make_shared<Network::TcpSocket>(sock_main);    // add it to the socklist map

  for ( int i=2; i<Flexure::N_THREADS; i++ ) {
    flexured.socklist[i] = std::make_shared<Network::TcpSocket>(sock_main);  // copy the first one, which has valid listening socket
    flexured.socklist[i]->id = i;                                            // update the id of the copied socket
  }

  // create the threads for the above sockets
  //
  for ( int i=1; i<Flexure::N_THREADS; i++ ) {
    std::thread( Flexure::Server::thread_main,
                 std::ref(flexured),
                 flexured.socklist[i] ).detach();
  }

  // Instantiate a multicast UDP object and spawn a thread to send asynchronous messages
  //
  Network::UdpSocket async(flexured.asyncport, flexured.asyncgroup);
  std::thread( std::ref(Flexure::Server::async_main),
               std::ref(flexured),
               async ).detach();

  // thread to start a new logbook each day
  //
  std::thread( std::ref(Flexure::Server::new_log_day), logpath ).detach();

  // Dynamically create a new listening socket and thread to handle each
  // connection request on the blocking port. Each socket will be added
  // to the socklist with an id from a pool of numbers. After that connection
  // is closed then its id will go back into the pool.
  //
  Network::TcpSocket sock_block(flexured.blkport, true, -1, 0);  // instantiate TcpSocket object with blocking port
  if ( sock_block.Listen() < 0 ) {                               // create a listening socket
    logwrite( function, "ERROR could not create listening socket" );
    flexured.exit_cleanly();
  }

  while (true) {
    auto newid = flexured.id_pool.get_next_number(); // get the next available number from the pool

    // lock the mutex before creating and initiating the new socket
    //
    {
    std::lock_guard<std::mutex> lock(flexured.sock_block_mutex);
    flexured.socklist[newid] = std::make_shared<Network::TcpSocket>(sock_block);  // create a new socket
    flexured.socklist[newid]->id = newid;            // set new socket's id
    flexured.socklist[newid]->Accept();              // accept connections on the new socket
    }

    // create a new thread to handle the connection
    //
    std::thread( Flexure::Server::block_main,
                 std::ref(flexured),
                 flexured.socklist[newid] ).detach();
  }

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
  std::string function = "Flexure::signal_handler";
  std::stringstream message;

  switch (signo) {
    case SIGTERM:
    case SIGINT:
      logwrite(function, "received termination signal");
      message << "NOTICE:" << Flexure::DAEMON_NAME << " exit";
      flexured.interface.async.enqueue( message.str() );
      flexured.exit_cleanly();                   // shutdown the daemon
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
      message.str(""); message << "NOTICE:" << Flexure::DAEMON_NAME << " exit";
      flexured.interface.async.enqueue( message.str() );
      flexured.exit_cleanly();                   // shutdown the daemon
      break;
  }
  return;
}
/***** signal_handler *********************************************************/
