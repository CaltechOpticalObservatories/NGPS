/**
 * @file    emulatord_tcs.cpp
 * @brief   this is the tcs emulator daemon
 * @details 
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#include "emulatord_tcs.h"
#include "daemonize.h"

/***** main *******************************************************************/
/**
 * @fn         main
 * @brief      the main function
 * @param[in]  int argc, char** argv
 * @return     0
 *
 */
int main( int argc, char **argv ) {
  std::string function = "  (TcsEmulator::main) ";
  std::stringstream message;
  bool start_daemon = true;

  // Allow running in the foreground
  //
  if ( cmdOptionExists( argv, argv+argc, "--foreground" ) ) {
    start_daemon = false;
  }

  // daemonize, but don't close all file descriptors
  //
  if ( start_daemon ) {
    std::cerr << get_timestamp() << function << " starting daemon\n";
    Daemon::daemonize( "emulatord.tcs", "/tmp", "/dev/null", "/tmp/emulatord.tcs.stderr", "", false );
    std::cerr << get_timestamp() << function << " daemonized. child process running\n";
  }

  // Now the child process instantiates a Server object
  //
  TcsEmulator::Server emulator;

  emulator.initialize_python_objects();
  PyEval_SaveThread();

  long ret=NO_ERROR;

  // check for "-f <filename>" command line option to specify config file
  //
  if ( cmdOptionExists( argv, argv+argc, "-f" ) ) {
    char* filename = getCmdOption( argv, argv+argc, "-f" );
    if ( filename ) {
      emulator.config.filename = std::string( filename );
    }
  }
  else

  // if no "-f <filename>" then as long as there's at least one arg,
  // assume that is the config file name.
  //
  if (argc>1) {
    emulator.config.filename = std::string( argv[1] );
  }
  else {
    std::cerr << get_timestamp() << function << emulator.subsystem
              << " ERROR: no configuration file specified\n";
    emulator.exit_cleanly();
  }

  if ( emulator.config.read_config(emulator.config) != NO_ERROR) {    // read configuration file specified on command line
    std::cerr << get_timestamp() << function << emulator.subsystem
              << " ERROR: unable to configure system\n";
    emulator.exit_cleanly();
  }

  for (int entry=0; entry < emulator.config.n_entries; entry++) {

    std::string configkey;          // the current configuration key read from file
    std::string configval;          // the current configuration value read from file
    
    try {
      configkey = emulator.config.param.at(entry);
      configval = emulator.config.arg.at(entry);
    }
    catch (std::out_of_range &) {   // should be impossible
      message.str(""); message << "ERROR: entry " << entry
                               << " out of range in config parameters (" << emulator.config.n_entries << ")";
      logwrite( function, message.str() );
      return(ERROR);
    }

  }

  std::cerr << get_timestamp() << function << emulator.subsystem << " " 
            << emulator.config.n_entries << " lines read from " << emulator.config.filename << "\n";

  ret = emulator.configure_emulator();    // get needed values out of read-in configuration file for the daemon
  if ( emulator.port < 1 ) {
    std::cerr << get_timestamp() << function << "ERROR: no port configured for the emulator\n";
    emulator.exit_cleanly();
  }

  if (ret != NO_ERROR) {
    std::cerr << get_timestamp() << function << "ERROR: unable to configure emulator for " << emulator.subsystem << "\n";
    emulator.exit_cleanly();
  }

  // TcpSocket objects are instantiated with (PORT#, BLOCKING_STATE, POLL_TIMEOUT_MSEC, THREAD_ID#)
  //
  std::vector<Network::TcpSocket> socklist;          // create a vector container to hold N_THREADS TcpSocket objects
  socklist.reserve(TcsEmulator::N_THREADS);

  // pre-thread N_THREADS detached threads to handle requests
  //
  for (int thrid=0; thrid<TcsEmulator::N_THREADS; thrid++) {      // create N_THREADS-1 non-blocking socket objects
    if (thrid==0) {                                  // first one only
      Network::TcpSocket s(emulator.port, true, -1, thrid); // instantiate TcpSocket object, blocking port, CONN_TIMEOUT timeout
      if ( s.Listen() < 0 ) {                        // create a listening socket
        std::cerr << get_timestamp() << function << "ERROR: cannot create listening socket on port " << emulator.port << "\n";
        emulator.exit_cleanly();
      }
      socklist.push_back(s);
    }
    else {                                           // subsequent socket objects are copies of the first
      Network::TcpSocket s = socklist[0];            // copy the first one, which has a valid listening socket
      s.id = thrid;
      socklist.push_back(s);
    }
    std::thread( std::ref(TcsEmulator::Server::block_main),
                 std::ref(emulator),
                 std::ref(socklist[thrid]) ).detach();   // spawn a thread to handle each non-blocking socket request
  }

  for (;;) pause();                                                // main thread suspends

  return 0;
}
/***** main *******************************************************************/
