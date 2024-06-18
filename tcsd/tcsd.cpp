/**
 * @file    tcsd.cpp
 * @brief   this is the main daemon for communicating with the TCS
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#include "tcsd.h"


/***** main *******************************************************************/
/**
 * @brief      the main function
 * @param[in]  argc  argument count
 * @param[in]  argv  character array that holds all arguments passed to the “main()” function through the command line
 * @return     0
 *
 */
int main(int argc, char **argv) {
  std::string function = "TCS::main";
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
      tcsd.config.filename = std::string( filename );
    }
  }
  else

  // if no "-f <filename>" then as long as there's at least one arg,
  // assume that is the config file name.
  //
  if (argc>1) {
    tcsd.config.filename = std::string( argv[1] );
  }
  else {
    logwrite(function, "ERROR: no configuration file specified");
    tcsd.exit_cleanly();
  }

  if ( tcsd.config.read_config(tcsd.config) != NO_ERROR) {          // read configuration file specified on command line
    logwrite(function, "ERROR: unable to configure system");
    tcsd.exit_cleanly();
  }

  for (int entry=0; entry < tcsd.config.n_entries; entry++) {
    if (tcsd.config.param[entry] == "LOGPATH") logpath = tcsd.config.arg[entry];
    if (tcsd.config.param[entry] == "DAEMON")  daemon_in = tcsd.config.arg[entry];

    if (tcsd.config.param[entry] == "TM_ZONE") {
      if ( tcsd.config.arg[entry] != "UTC" && tcsd.config.arg[entry] != "local" ) {
        message.str(""); message << "ERROR invalid TM_ZONE=" << tcsd.config.arg[entry] << ": expected UTC|local";
        logwrite( function, message.str() );
        tcsd.exit_cleanly();
      }
      tmzone_cfg = tcsd.config.arg[entry];
      message.str(""); message << "config:" << tcsd.config.param[entry] << "=" << tcsd.config.arg[entry];
      logwrite( function, message.str() );
      tcsd.interface.async.enqueue( message.str() );
    }

  }

  if (logpath.empty()) {
    logwrite(function, "ERROR: LOGPATH not specified in configuration file");
    tcsd.exit_cleanly();
  }

  if ( !daemon_in.empty() && daemon_in == "yes" ) start_daemon = true;
  else
  if ( !daemon_in.empty() && daemon_in == "no"  ) start_daemon = false;
  else {
    message.str(""); message << "ERROR: unrecognized argument DAEMON=" << daemon_in << ", expected { yes | no }";
    logwrite( function, message.str() );
    tcsd.exit_cleanly();
  }

  // check for "-d" command line option last so that the command line
  // can override the config file to start as daemon
  //
  if ( cmdOptionExists( argv, argv+argc, "-d" ) ) {
    start_daemon = true;
  }

  if ( start_daemon ) {
    logwrite( function, "starting daemon" );
    Daemon::daemonize( TCS::DAEMON_NAME, "/tmp", "", "", "" );
  }

  if ( ( init_log( logpath, TCS::DAEMON_NAME ) != 0 ) ) {            // initialize the logging system
    logwrite(function, "ERROR: unable to initialize logging system");
    tcsd.exit_cleanly();
  }

  message << "this version built " << BUILD_DATE << " " << BUILD_TIME;
  logwrite(function, message.str());

  message.str(""); message << tcsd.config.n_entries << " lines read from " << tcsd.config.filename;
  logwrite(function, message.str());

  if (ret==NO_ERROR) ret=tcsd.configure_tcsd();          // get needed values out of read-in configuration file for the daemon

  if (ret != NO_ERROR) {
    logwrite(function, "ERROR: unable to configure system");
    tcsd.exit_cleanly();
  }

  if (tcsd.nbport == -1 || tcsd.blkport == -1) {
    logwrite(function, "ERROR: tcsd ports not configured");
    tcsd.exit_cleanly();
  }

  // This will pre-thread N_THREADS threads, a little differently from other
  // daemons.  There will be N_THREADS-1 non-blocking threads as before then
  // loop forever on Accept to dynamically spawn a new thread for each blocking
  // port connection (released when client disconnects).
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
  Network::TcpSocket sock_main(tcsd.nbport, false, CONN_TIMEOUT, 1);     // instantiate TcpSocket object, non-blocking port, CONN_TIMEOUT timeout
  if ( sock_main.Listen() < 0 ) {                                        // create a listening socket
    logwrite( function, "ERROR could not create listening socket" );
    tcsd.exit_cleanly();
  }
  tcsd.socklist[1] = std::make_shared<Network::TcpSocket>(sock_main);    // add it to the socklist map

  for (int i = 2; i < TCS::N_THREADS; i++) {
    tcsd.socklist[i] = std::make_shared<Network::TcpSocket>(sock_main);  // copy the first one, which has a valid listening socket
    tcsd.socklist[i]->id = i;                                            // update the id of the copied socket
  }

  // Create the threads for the above sockets
  //
  for ( int i = 1; i < TCS::N_THREADS; i++) {
    std::thread( TCS::Server::thread_main,
                 std::ref(tcsd),
                 tcsd.socklist[i] ).detach();
  }

  // Instantiate a multicast UDP object and spawn a thread to send asynchronous messages
  //
  Network::UdpSocket async(tcsd.asyncport, tcsd.asyncgroup);
  std::thread( std::ref(TCS::Server::async_main),
               std::ref(tcsd),
               async ).detach();

  // thread to start a new logbook each day
  //
  std::thread( std::ref(TCS::Server::new_log_day), logpath ).detach();

  // Dynamically create a new listening socket and thread to handle
  // each connection request on the blocking port.
  //
  Network::TcpSocket sock_block(tcsd.blkport, true, -1, 0);              // instantiate TcpSocket object with blocking port
  if ( sock_block.Listen() < 0 ) {                                       // create a listening socket
    logwrite( function, "ERROR could not create listening socket" );
    tcsd.exit_cleanly();
  }

  while (true) {
    auto newid = tcsd.id_pool.get_next_number();  // get the next available number from the pool
    
    // Lock the mutex before creating and initializing the new socket
    //
    {
    std::lock_guard<std::mutex> lock(tcsd.sock_block_mutex);
    tcsd.socklist[newid] = std::make_shared<Network::TcpSocket>(sock_block);  // create a new socket
    tcsd.socklist[newid]->id = newid;        // update the id of the copied socket
    tcsd.socklist[newid]->Accept();          // accept connections on the new socket
    }

    // Create a new thread to handle the connection
    //
    std::thread(TCS::Server::block_main, std::ref(tcsd), tcsd.socklist[newid]).detach();
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
  std::string function = "TCS::signal_handler";
  std::stringstream message;

  switch (signo) {
    case SIGTERM:
    case SIGINT:
      logwrite(function, "received termination signal");
      message << "NOTICE:" << TCS::DAEMON_NAME << " exit";
      tcsd.interface.async.enqueue( message.str() );
      tcsd.exit_cleanly();                       // shutdown the daemon
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
      message.str(""); message << "NOTICE:" << TCS::DAEMON_NAME << " exit";
      tcsd.interface.async.enqueue( message.str() );
      tcsd.exit_cleanly();                       // shutdown the daemon
      break;
  }
  return;
}
/***** signal_handler *********************************************************/
