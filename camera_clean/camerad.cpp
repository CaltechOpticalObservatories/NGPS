/**
 * @file    camerad.cpp
 * @brief   this is the main daemon for communicating with the camera
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#include "camerad.h"
#include "daemon_config.h"
#include "camera_interface.h"

/***** main *******************************************************************/
/**
 * @brief      the main function
 * @param[in]  argc  argument count
 * @param[in]  argv  array holds arguments passed on command line
 * @return     0
 *
 */
int main( int argc, char** argv ) {
  std::string function("Camera::main");
  std::stringstream message;
  bool start_daemon = true;
  long error = NO_ERROR;

  // Allow running in the foreground
  //
  if ( cmdOptionExists( argv, argv+argc, "--foreground" ) ) {
    start_daemon = false;
  }

  // daemonize
  //
  if (start_daemon) {
    logwrite(function, "starting daemon");
    Daemon::daemonize( Camera::Daemon::NAME,
                       "/tmp",
                       Camera::Daemon::STDOUT,
                       Camera::Daemon::STDERR,
                       "",
                       false );
    logwrite(function, "daemonized. child process running");
  }

  // the child process instantiates a Server object
  //
  Camera::Server camerad;

  // get config file name from "--config <filename>" command line option
  //
  std::string configfilename = getCmdOption(argv, argv+argc, "--config");

  // assign configuration file
  //
  if (!configfilename.empty()) {
    camerad.config.filename = configfilename;
  }
  else
  // if no "--config <filename>" then as long as there's at least one arg,
  // assume that is the config file name.
  //
  if (argc>1) {
    camerad.config.filename = std::string( argv[1] );
  }
  else {
    logwrite(function, "ERROR (fatal) no configuration file specified");
    camerad.exit_cleanly();
  }

  // read configuration file
  //
  if ( camerad.config.read_config(camerad.config) != NO_ERROR) {
    logwrite(function, "ERROR (fatal) unable to read configuration file");
    camerad.exit_cleanly();
  }

  // A few config keys are parsed here
  //
  for (int entry=0; entry < camerad.config.n_entries; entry++) {

    // LOGPATH
    if (camerad.config.param[entry] == "LOGPATH") {
      std::string logpath(camerad.config.arg[entry]);
      if (logpath.empty()) {
        logwrite(function, "ERROR (fatal) LOGPATH not specified in configuration file");
        camerad.exit_cleanly();
      }
      if ( (init_log(logpath, Camera::Daemon::NAME) != 0) ) {
        logwrite(function, "ERROR (fatal) unable to initialize logging system");
        camerad.exit_cleanly();
      }
      // thread to stsart a new logbook each day
      std::thread( &Camera::Server::new_log_day, &camerad, logpath ).detach();
    }

    // TM_ZONE
    if (camerad.config.param[entry] == "TM_ZONE") {
      if ( camerad.config.arg[entry] != "UTC" && camerad.config.arg[entry] != "local" ) {
        message.str(""); message << "ERROR invalid TM_ZONE=" << camerad.config.arg[entry] << ": expected UTC|local";
        logwrite( function, message.str() );
        camerad.exit_cleanly();
      }
      tmzone_cfg = camerad.config.arg[entry];
      message.str(""); message << "config:" << camerad.config.param[entry] << "=" << camerad.config.arg[entry];
      logwrite( function, message.str() );
    }
  }

  // log build date and hash
  //
  message << "this version built " << BUILD_DATE << " " << BUILD_TIME;
  logwrite(function, message.str());

  message.str(""); message << camerad.config.n_entries << " lines read from " << camerad.config.filename;
  logwrite(function, message.str());

  // finish parsing the configuration file for ...
  //
  if (error==NO_ERROR) error=camerad.configure_server();                // ...the server
  if (error==NO_ERROR) error=camerad.interface->configure_camera();     // ...the camera
  if (error==NO_ERROR) error=camerad.interface->configure_constkeys();  // ...constant FITS keys

  if (error != NO_ERROR) {
    logwrite(function, "ERROR (fatal) unable to configure system");
    camerad.exit_cleanly();
  }

  if (camerad.nbport == 0 || camerad.blkport == 0) {
    logwrite(function, "ERROR (fatal) camerad ports not configured");
    camerad.exit_cleanly();
  }

  // testing
  //
  // camerad.interface->myfunction();

  // dynamically create a new listening socket and thread
  // to handle each connection request
  //
  Network::TcpSocket sock_block(8675, true, -1, 0);  // instantiate a TcpSocket with blocking port
  if ( sock_block.Listen() < 0 ) {
    std::cerr << "ERROR could not create listening socket\n";
    exit(1);
  }

  while ( true ) {
    auto newid = camerad.id_pool.get_next_number();  // get next available number from the pool
    {
    std::lock_guard<std::mutex> lock(camerad.sock_block_mutex);
    camerad.socklist[newid] = std::make_shared<Network::TcpSocket>(sock_block);  // create a new socket
    camerad.socklist[newid]->id = newid;             // update the id of the copied socket
    camerad.socklist[newid]->Accept();               // accept connections on the new socket
    }

    // Create a new thread to handle the connection
    //
    std::thread( &Camera::Server::block_main, &camerad, camerad.socklist[newid] ).detach();
  }

  for (;;) pause();                                  // main thread suspends

  return 0;
}

