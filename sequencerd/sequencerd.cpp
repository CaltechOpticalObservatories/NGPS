/**
 * @file    sequencerd.cpp
 * @brief   this is the main observation sequencer daemon
 * @author  David Hale <dhale@astro.caltech.edu>
 * @details The observation sequencer orchestrates all subsystems required to carry out an observation.
 *
 * This file has the required global function main()
 *
 */

#include "sequencerd.h"


/***** main *******************************************************************/
/**
 * @brief      the main function
 * @param[in]  argc  argument count
 * @param[in]  argv  character array that holds all arguments passed to the “main()” function through the command line
 * @return     0
 *
 */
int main(int argc, char **argv) {
  std::string function = "Sequencer::main";
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
  std::string daemon_stderr="/tmp/"+Sequencer::DAEMON_NAME+".stderr";  // where daemon sends stderr

  // daemonize, but don't close all file descriptors
  //
  if ( start_daemon ) {
    logwrite( function, "starting daemon" );
    Daemon::daemonize( Sequencer::DAEMON_NAME, "/tmp", daemon_stdout, daemon_stderr, "", false );
  }

  logwrite( function, "daemonized. child process running" );

  // Now the child process instantiates a Server object so that
  // the daemon can access the namespace.
  //
  Sequencer::Server sequencerd;

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
      sequencerd.config.filename = std::string( filename );
    }
  }
  else

  // if no "-f <filename>" then as long as there's at least one arg,
  // assume that is the config file name.
  //
  if (argc>1) {
    sequencerd.config.filename = std::string( argv[1] );
  }
  else {
    logwrite(function, "ERROR: no configuration file specified");
    sequencerd.exit_cleanly();
  }

  if ( sequencerd.config.read_config(sequencerd.config) != NO_ERROR) {  // read configuration file specified on command line
    logwrite(function, "ERROR: unable to configure system");
    sequencerd.exit_cleanly();
  }

  for (int entry=0; entry < sequencerd.config.n_entries; entry++) {
    if (sequencerd.config.param[entry] == "LOGPATH") logpath = sequencerd.config.arg[entry];

    if (sequencerd.config.param[entry] == "TM_ZONE") {
      if ( sequencerd.config.arg[entry] != "UTC" && sequencerd.config.arg[entry] != "local" ) {
        message.str(""); message << "ERROR invalid TM_ZONE=" << sequencerd.config.arg[entry] << ": expected UTC|local";
        logwrite( function, message.str() );
        sequencerd.exit_cleanly();
      }
      tmzone_cfg = sequencerd.config.arg[entry];
      message.str(""); message << "config:" << sequencerd.config.param[entry] << "=" << sequencerd.config.arg[entry];
      logwrite( function, message.str() );
    }

  }

  if (logpath.empty()) {
    logwrite(function, "ERROR: LOGPATH not specified in configuration file");
    sequencerd.exit_cleanly();
  }

  if ( ( init_log( logpath, Sequencer::DAEMON_NAME ) != 0 ) ) {      // initialize the logging system
    logwrite(function, "ERROR: unable to initialize logging system");
    sequencerd.exit_cleanly();
  }

  message << "this version built " << BUILD_DATE << " " << BUILD_TIME;
  logwrite(function, message.str());

  message.str(""); message << sequencerd.config.n_entries << " lines read from " << sequencerd.config.filename;
  logwrite(function, message.str());

  if (ret==NO_ERROR) ret=sequencerd.configure_sequencer();   // get needed values out of read-in configuration file for the sequencer

  if (ret != NO_ERROR) {
    logwrite(function, "ERROR: unable to configure system");
    sequencerd.exit_cleanly();
  }

  if (sequencerd.nbport == -1 || sequencerd.blkport == -1) {
    logwrite(function, "ERROR: sequencer ports not configured");
    sequencerd.exit_cleanly();
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
  socklist.reserve(Sequencer::N_THREADS);

  Network::TcpSocket s(sequencerd.blkport, true, -1, 0); // instantiate TcpSocket object with blocking port
  if ( s.Listen() < 0 ) {                            // create a listening socket
    logwrite( function, "ERROR could not create listening socket" );
    sequencerd.exit_cleanly();
  }
  socklist.push_back(s);                             // add it to the socklist vector
  std::thread( std::ref(Sequencer::Server::block_main),
               std::ref(sequencerd),
               std::ref(socklist[0]) ).detach();     // spawn a thread to handle requests on this socket

  // pre-thread N_THREADS-1 detached threads to handle requests on the non-blocking port
  // thread #0 is reserved for the blocking port (above)
  //
  int thrid=1;
  for (thrid=1; thrid<Sequencer::N_THREADS-1; thrid++) {        // create N_THREADS-1 non-blocking socket objects
    if (thrid==1) {                                  // first one only
      Network::TcpSocket s(sequencerd.nbport, false, 0, thrid); // instantiate TcpSocket object, non-blocking port, CONN_TIMEOUT timeout
      if ( s.Listen() < 0 ) {                        // create a listening socket
        logwrite( function, "ERROR could not create listening socket" );
        sequencerd.exit_cleanly();
      }
      socklist.push_back(s);
    }
    else {                                           // subsequent socket objects are copies of the first
      Network::TcpSocket s = socklist[1];            // copy the first one, which has a valid listening socket
      s.id = thrid;
      socklist.push_back(s);
    }
    std::thread( std::ref(Sequencer::Server::thread_main),
                 std::ref(sequencerd),
                 std::ref(socklist[thrid]) ).detach();   // spawn a thread to handle each non-blocking socket request
  }

  Network::TcpSocket async( Sequencer::DAEMON_NAME, sequencerd.asyncport, true, true, -1, thrid );    // instantiate TcpSocket object as blocking and asynchronous
  async.Listen();                                    // create a listening socket
  socklist.push_back(async);                         // add it to the socklist vector
  std::thread( std::ref(Sequencer::Server::block_main),
               std::ref(sequencerd),
               std::ref(socklist[thrid]) ).detach(); // spawn a thread to handle requests on this socket

  // Instantiate a multicast UDP object and spawn a thread to send asynchronous messages
  //
  Network::UdpSocket msg(sequencerd.messageport, sequencerd.messagegroup);
  std::thread( std::ref(Sequencer::Server::async_main),
               std::ref(sequencerd),
               msg ).detach();

  // Create my own asynchronous listener thread.
  // This thread allows the sequencer to listen for asynchronous messages.
  //
  std::thread( std::ref( Sequencer::Sequence::dothread_sequencer_async_listener ), 
               std::ref( sequencerd.sequence),
               msg
             ).detach();

//std::thread( std::ref( Sequencer::Sequence::dothread_monitor_ready_state ), 
//             std::ref( sequencerd.sequence)
//           ).detach();

  // thread to start a new logbook each day
  //
  std::thread( std::ref(Sequencer::Server::new_log_day), logpath ).detach();

  sequencerd.sequence.async.enqueue( "SEQUENCERD:started" );  // broadcast that I have started

  sequencerd.sequence.report_seqstate();                      // broadcast the seqstate

  for (;;) pause();                                  // main thread suspends

  return 0;
}
/***** main *******************************************************************/


/***** signal_handler *********************************************************/
/**
 * @brief  handles ctrl-C
 * @param  int signo
 * @return nothing
 *
 */
/***
void signal_handler(int signo) {
  std::string function = "Sequencer::signal_handler";
  switch (signo) {
    case SIGTERM:
    case SIGINT:
      logwrite(function, "received termination signal");
      sequencerd.exit_cleanly();                 // shutdown the sequencer
      break;
    case SIGHUP:
      logwrite(function, "caught SIGHUP");
      sequencerd.configure_sequencer();          // TODO can (/should) this be done while running?
      break;
    case SIGPIPE:
      logwrite(function, "caught SIGPIPE");
      break;
    default:
      logwrite(function, "received unknown signal");
      sequencerd.exit_cleanly();                 // shutdown the sequencer
      break;
  }
  return;
}
***/
/***** signal_handler *********************************************************/
