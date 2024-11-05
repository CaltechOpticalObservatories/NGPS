/**
 * @file    camerad.cpp
 * @brief   this is the main camerad server
 * @details spawns threads to handle requests, receives and parses commands
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#include "build_date.h"
#include "camerad.h"
#include "daemonize.h"

Camera::Server server;
std::string logpath; 

/***** signal_handler *********************************************************/
/**
 * @brief      handles ctrl-C
 * @param[in]  signo
 *
 */
void signal_handler(int signo) {
  std::string function = "Camera::signal_handler";
  switch (signo) {
    case SIGTERM:
    case SIGINT:
      logwrite(function, "received termination signal");
      server.camera.async.enqueue("exit");  // shutdown the async_main thread if running
      server.exit_cleanly();                // shutdown the server
      break;
    case SIGHUP:
      logwrite(function, "caught SIGHUP");
      server.configure_controller();
      break;
    case SIGPIPE:
      logwrite(function, "caught SIGPIPE");
      break;
    default:
      logwrite(function, "received unknown signal");
      server.camera.async.enqueue("exit");  // shutdown the async_main thread if running
      server.exit_cleanly();                // shutdown the server
      break;
  }
  return;
}
/***** signal_handler *********************************************************/


int  main(int argc, char **argv);           // main thread (just gets things started)
void new_log_day( );                        // create a new log each day
void block_main(Network::TcpSocket sock);   // this thread handles requests on blocking port
void thread_main(Network::TcpSocket &sock); // this thread handles requests on non-blocking port
void async_main(Network::UdpSocket sock);   // this thread handles the asyncrhonous UDP message port
void doit(Network::TcpSocket &sock);        // the worker thread


/***** main *******************************************************************/
/**
 * @brief      the main function
 * @param[in]  argc  argument count
 * @param[in]  argv  character array that holds all arguments passed to the “main()” function through the command line
 * @return     0
 *
 */
int main(int argc, char **argv) {
  std::string function = "Camera::main";
  std::stringstream message;
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
      server.config.filename = std::string( filename );
    }
  }
  else

  // if no "-f <filename>" then as long as there's at least one arg,
  // assume that is the config file name.
  //
  if (argc>1) {
    server.config.filename = std::string( argv[1] );
  }
  else {
    logwrite(function, "ERROR: no configuration file specified");
    server.exit_cleanly();
  }

  if ( server.config.read_config(server.config) != NO_ERROR) {  // read configuration file specified on command line
    logwrite(function, "ERROR: unable to configure system");
    server.exit_cleanly();
  }

  for (int entry=0; entry < server.config.n_entries; entry++) {
    if (server.config.param[entry] == "LOGPATH") logpath = server.config.arg[entry];    // where to write log files
    if (server.config.param[entry] == "DAEMON")  daemon_in = server.config.arg[entry];  // am I starting as a daemon or not?

    if (server.config.param[entry] == "TM_ZONE") {
      if ( server.config.arg[entry] != "UTC" && server.config.arg[entry] != "local" ) {
        message.str(""); message << "ERROR invalid TM_ZONE=" << server.config.arg[entry] << ": expected UTC|local";
        logwrite( function, message.str() );
        server.exit_cleanly();
      }
      tmzone_cfg = server.config.arg[entry];
      server.camera_info.systemkeys.primary().addkey( "TM_ZONE", tmzone_cfg, "time zone" );  // add to primary system keys database
      message.str(""); message << "config:" << server.config.param[entry] << "=" << server.config.arg[entry];
      logwrite( function, message.str() );
      server.camera.async.enqueue( message.str() );
    }

  }

  if (logpath.empty()) {
    logwrite(function, "ERROR: LOGPATH not specified in configuration file");
    server.exit_cleanly();
  }

  if ( !daemon_in.empty() && daemon_in == "yes" ) start_daemon = true;
  else
  if ( !daemon_in.empty() && daemon_in == "no"  ) start_daemon = false;
  else
  if ( !daemon_in.empty() ) {
    message.str(""); message << "ERROR: unrecognized argument DAEMON=" << daemon_in << ", expected { yes | no }";
    logwrite( function, message.str() );
    server.exit_cleanly();
  }

  // check for "-d" command line option last so that the command line
  // can override the config file to start as daemon
  //
  if ( cmdOptionExists( argv, argv+argc, "-d" ) ) {
    start_daemon = true;
  }

  if ( start_daemon ) {
    logwrite( function, "starting daemon" );
    Daemon::daemonize( Camera::DAEMON_NAME, "/tmp", "", "", "" );
  }

  if ( ( init_log( logpath, Camera::DAEMON_NAME ) != 0 ) ) {         // initialize the logging system
    logwrite(function, "ERROR: unable to initialize logging system");
    server.exit_cleanly();
  }

  // log build date and hash
  //
  message.str(""); message << "this version built " << BUILD_DATE << " " << BUILD_TIME;
  logwrite( function, message.str() );

  message.str(""); message << server.config.n_entries << " lines read from " << server.config.filename;
  logwrite(function, message.str());

  if (ret==NO_ERROR) ret=server.configure_server();      // get needed values out of read-in configuration file for the server
  if (ret==NO_ERROR) ret=server.configure_controller();  // get needed values out of read-in configuration file for the controller
  if (ret==NO_ERROR) ret=server.configure_constkeys();   // get constant FITS keys out of read-in configuration file

  if (ret != NO_ERROR) {
    logwrite(function, "ERROR: unable to configure system");
    server.exit_cleanly();
  }

  if (server.nbport == -1 || server.blkport == -1) {
    logwrite(function, "ERROR: server ports not configured");
    server.exit_cleanly();
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

  Network::TcpSocket s(server.blkport, true, -1, 0); // instantiate TcpSocket object with blocking port
  s.Listen();                                        // create a listening socket
  socklist.push_back(s);                             // add it to the socklist vector
  std::thread(block_main, socklist[0]).detach();     // spawn a thread to handle requests on this socket

  // pre-thread N_THREADS-1 detached threads to handle requests on the non-blocking port
  // thread #0 is reserved for the blocking port (above)
  //
  for (int i=1; i<N_THREADS; i++) {                  // create N_THREADS-1 non-blocking socket objects
    if (i==1) {                                      // first one only
      Network::TcpSocket s(server.nbport, false, 0, i);   // instantiate TcpSocket object, non-blocking port, CONN_TIMEOUT timeout
      s.Listen();                                    // create a listening socket
      socklist.push_back(s);
    }
    else {                                           // subsequent socket objects are copies of the first
      Network::TcpSocket s = socklist[1];            // copy the first one, which has a valid listening socket
      s.id = i;
      socklist.push_back(s);
    }
    std::thread(thread_main, std::ref(socklist[i]) ).detach();  // spawn a thread to handle each non-blocking socket request
  }

  // Instantiate a multicast UDP object and spawn a thread to send asynchronous messages
  //
  Network::UdpSocket async(server.asyncport, server.asyncgroup);
  std::thread(async_main, async).detach();

  // thread to start a new logbook each day
  //
  std::thread( new_log_day ).detach();

  for (;;) pause();                                  // main thread suspends
  return 0;
}
/***** main *******************************************************************/


/***** new_log_day ************************************************************/
/**
 * @brief      creates a new logbook each day
 *
 * This thread is started by main and never terminates.
 * It sleeps for the number of seconds that logentry determines
 * are remaining in the day, then closes and re-inits a new log file.
 *
 * The number of seconds until the next day "nextday" is a global which
 * is set by init_log.
 *
 */
void new_log_day( ) { 
  while (1) {
    std::this_thread::sleep_for( std::chrono::seconds( nextday ) );
    close_log();
    init_log( logpath, Camera::DAEMON_NAME );
  }
}
/***** new_log_day ************************************************************/


/***** block_main *************************************************************/
/**
 * @brief      main function for blocking connection thread
 * @param[in]  sock  Network::TcpSocket socket object
 *
 * accepts a socket connection and processes the request by
 * calling function doit()
 *
 * This thread never terminates.
 *
 */
void block_main(Network::TcpSocket sock) {
  while(1) {
    sock.Accept();
    doit(sock);                   // call function to do the work
    sock.Close();
  }
  return;
}
/***** block_main *************************************************************/


/***** Server::thread_main ****************************************************/
/**
 * @brief      main function for all non-blocked threads
 * @param[in]  seq   reference to Sequencer::Server object
 * @param[in]  sock  reference to Network::TcpSocket socket object
 *
 * accepts a socket connection and processes the request by
 * calling function doit()
 *
 * There are N_THREADS-1 of these, one for each non-blocking connection.
 * These threads never terminate.
 *
 * This function differs from block_main in that the call to Accept
 * is mutex-protected, and the sock object is passed in by reference,
 * which allows the doit function to be spawned as a separate thread.
 *
 */
void thread_main(Network::TcpSocket &sock) {
  while (1) {
    {
    std::lock_guard<std::mutex> lock(server.conn_mutex);
    sock.Accept();
    }
#ifdef LOGLEVEL_DEBUG
    std::stringstream message;
    message.str(""); message << "[DEBUG] thread " << sock.id
                             << " spawning doit to handle connection on fd " << sock.getfd();
    logwrite( "Server::thread_main", message.str() );
#endif
    std::thread(doit, std::ref(sock)).detach();  // spawn a thread to handle this connection
  }
  return;
}
/***** Server::thread_main ****************************************************/


/***** Server::async_main ***************************************************/
/**
 * @brief      asynchronous message sending thread
 * @param[in]  sock  Network::udpSocket socket object
 * Loops forever, when a message arrives in the status message queue it is
 * sent out via multi-cast UDP datagram.
 *
 */
void async_main(Network::UdpSocket sock) {
  std::string function = "Camera::async_main";
  int retval;

  retval = sock.Create();                                   // create the UDP socket
  if (retval < 0) {
    logwrite(function, "error creating UDP multicast socket for asynchronous messages");
    server.exit_cleanly();                                  // do not continue on error
  }
  if (retval==1) {                                          // exit this thread but continue with server
    logwrite(function, "asyncrhonous message port disabled by request");
  }

  while (1) {
    std::string message = server.camera.async.dequeue();    // get the latest message from the queue (blocks)
    retval = sock.Send(message);                            // transmit the message
    if (retval < 0) {
      std::stringstream errstm;
      errstm << "error sending UDP message: " << message;
      logwrite(function, errstm.str());
    }
    if (message=="exit") {                                  // terminate this thread
      sock.Close();
      return;
    }
  }
  return;
}
/***** Server::async_main ***************************************************/


/***** Server::doit *********************************************************/
/**
 * @brief      the workhorse of each thread connetion
 * @param[in]  sock  reference to Network::UdpSocket socket object
 *
 * stays open until closed by client
 *
 * commands come in the form: 
 *   "<device> [all|<app>] [_BLOCK_] <command> [<arg>]"
 *
 */
void doit(Network::TcpSocket &sock) {
  std::string function = "Camera::doit";
  char  buf[BUFSIZE+1];
  long  ret;
  std::stringstream message;
  std::string cmd, args;        // arg string is everything after command
  std::vector<std::string> tokens;

  bool connection_open=true;

  message.str(""); message << "thread " << sock.id << " accepted connection on fd " << sock.getfd();
  logwrite( function, message.str() );

  while (connection_open) {
    memset(buf,  '\0', BUFSIZE);  // init buffers

    // Wait (poll) connected socket for incoming data...
    //
    int pollret;
    if ( ( pollret=sock.Poll() ) <= 0 ) {
      if (pollret==0) {
        message.str(""); message << "Poll timeout on fd " << sock.getfd() << " thread " << sock.id;
        logwrite(function, message.str());
      }
      if (pollret <0) {
        message.str(""); message << "Poll error on fd " << sock.getfd() << " thread " << sock.id << ": " << strerror(errno);
        logwrite(function, message.str());
      }
      break;                      // this will close the connection
    }

    // Data available, now read from connected socket...
    //
    std::string sbuf;
    if ( ( ret=sock.Read( sbuf, '\n' ) ) <= 0 ) {
      if (ret<0) {                // could be an actual read error
        message.str(""); message << "Read error on fd " << sock.getfd() << ": " << strerror(errno); logwrite(function, message.str());
      }
      if (ret==0) {
        message.str(""); message << "timeout reading from fd " << sock.getfd();
        logwrite( function, message.str() );
      }
      break;                      // Breaking out of the while loop will close the connection.
                                  // This probably means that the client has terminated abruptly, 
                                  // having sent FIN but not stuck around long enough
                                  // to accept CLOSE and give the LAST_ACK.
    }

    // convert the input buffer into a string and remove any trailing linefeed
    // and carriage return
    //
    sbuf.erase(std::remove(sbuf.begin(), sbuf.end(), '\r' ), sbuf.end());
    sbuf.erase(std::remove(sbuf.begin(), sbuf.end(), '\n' ), sbuf.end());

    if ( sbuf.empty() ) sbuf="help";                 // no command automatically displays help

    try {
      std::size_t cmd_sep = sbuf.find_first_of(" "); // find the first space, which separates command from argument list

      cmd = sbuf.substr(0, cmd_sep);                 // cmd is everything up until that space

      if (cmd.empty()) {sock.Write("\n"); continue;} // acknowledge empty command so client doesn't time out

      if (cmd_sep == std::string::npos) {            // If no space was found,
        args="";                                     // then the arg list is empty,
      }
      else {
        args= sbuf.substr(cmd_sep+1);                // otherwise args is everything after that space.
      }

      if ( ++server.cmd_num == INT_MAX ) server.cmd_num = 0;

      message.str(""); message << "thread " << sock.id << " received command on fd " << sock.getfd() << " (" << server.cmd_num << ") : " << cmd << " " << args;
      logwrite(function, message.str());
    }
    catch ( std::runtime_error &e ) {
      std::stringstream errstream; errstream << e.what();
      message.str(""); message << "error parsing arguments: " << errstream.str();
      logwrite(function, message.str());
      ret = -1;
    }
    catch ( ... ) {
      message.str(""); message << "unknown error parsing arguments: " << args;
      logwrite(function, message.str());
      ret = -1;
    }

    /**
     * process commands here
     */
    ret = NOTHING;
    std::string retstring="";                               // string for return the value (where needed)

    if ( cmd == "-h" || cmd == "--help" || cmd == "help" || cmd == "?" ) {
                    retstring="camera { <CMD> } [<ARG>...]\n";
                    retstring.append( "  where <CMD> is one of:\n" );
                    for ( const auto &s : CAMERAD_SYNTAX ) {
                      retstring.append("  "); retstring.append( s ); retstring.append( "\n" );
                    }
                    ret = HELP;
    }
    else
    if ( cmd == "exit" ) {
                    server.camera.async.enqueue("exit");    // shutdown the async message thread if running
                    server.exit_cleanly();                  // shutdown the server
                    }
    else
    if ( cmd == CAMERAD_CONFIG ) {              // report the config file used for camerad
                    std::stringstream cfg;
                    cfg << "CONFIG:" << server.config.filename;
                    server.camera.async.enqueue( cfg.str() );
                    sock.Write( server.config.filename );
                    sock.Write( " " );
                    ret = NO_ERROR;
                    }
    // send telemetry as json message
    //
    if ( cmd == TELEMREQUEST ) {
                    if ( args=="?" || args=="help" ) {
                      retstring=TELEMREQUEST+"\n";
                      retstring.append( "  Returns a serialized JSON message containing my telemetry\n" );
                      retstring.append( "  information, terminated with \"EOF\\n\".\n" );
                      ret=HELP;
                    }
                    else {
                      server.make_telemetry_message( retstring );
                      ret = JSON;
                    }
    }
    else
    if ( cmd == CAMERAD_OPEN ) {
                    ret = server.connect_controller(args, retstring);
                    }
    else
    if ( cmd == CAMERAD_CLOSE ) {
                    ret = server.disconnect_controller();
                    }
    else
    if ( cmd == CAMERAD_LOAD ) {
                    if (args.empty()) ret = server.load_firmware(retstring);
                    else              ret = server.load_firmware(args, retstring);
                    if (!retstring.empty()) { sock.Write(retstring); sock.Write(" "); }
                    }
    else
    if ( cmd == CAMERAD_BASENAME ) {
                    ret = server.camera.basename(args, retstring);
                    sock.Write(retstring);
                    sock.Write(" ");
                    }
    else
    if ( cmd == CAMERAD_IMNUM ) {
                    ret = server.camera.imnum(args, retstring);
                    if (!retstring.empty()) { sock.Write(retstring); sock.Write(" "); }
                    }
    else
    if ( cmd == CAMERAD_IMDIR ) {
                    ret = server.camera.imdir(args, retstring);
                    sock.Write(retstring);
                    sock.Write(" ");
                    }
    else
    if ( cmd == CAMERAD_AUTODIR ) {
                    ret = server.camera.autodir(args, retstring);
                    if (!retstring.empty()) { sock.Write(retstring); sock.Write(" "); }
                    }
    else
    if ( cmd == CAMERAD_MEX ) {
                    ret = server.camera.mex(args, retstring);
                    sock.Write(retstring);
                    sock.Write(" ");
                    }
    else
    if ( cmd == CAMERAD_LONGERROR ) {
                    ret = server.camera.longerror(args, retstring);
                    sock.Write(retstring);
                    sock.Write(" ");
                    }
    else
    if ( cmd == CAMERAD_PREEXPOSURES ) {
                    ret = server.camera_info.pre_exposures( args, retstring );
                    sock.Write( retstring );
                    sock.Write( " " );
                    }
    else
    if ( cmd == CAMERAD_MEXAMPS ) {
                    ret = server.camera.mexamps(args, retstring);
                    sock.Write(retstring);
                    sock.Write(" ");
                    }
    else
    if ( cmd == CAMERAD_FITSNAMING ) {
                    ret = server.camera.fitsnaming(args, retstring);
                    }
    else
    if ( cmd == CAMERAD_SHUTTER ) {
                    ret = server.shutter(args, retstring);
                    }
    else
    if ( cmd == CAMERAD_WRITEKEYS ) {
                    ret = server.camera.writekeys(args, retstring);
                    }
    else
    if ( cmd == CAMERAD_KEY ) {
                    if ( args.substr(0, 4) == "list" ) {
                      for ( std::size_t i=0; i<server.fitsinfo.size(); ++i ) {
                        const auto &info = server.fitsinfo[i];
                        message.str(""); message << "fitsinfo[" << i << "] primary systemkeys:"; logwrite( function, message.str() );
                        ret = info != nullptr && info->systemkeys.primary().listkeys();
                        message.str(""); message << "fitsinfo[" << i << "] primary telemkeys:"; logwrite( function, message.str() );
                        ret = info != nullptr && info->telemkeys.primary().listkeys();
                        message.str(""); message << "fitsinfo[" << i << "] extension systemkeys:"; logwrite( function, message.str() );
                        ret = info != nullptr && info->systemkeys.extension().listkeys();
                        message.str(""); message << "fitsinfo[" << i << "] extension telemkeys:"; logwrite( function, message.str() );
                        ret = info != nullptr && info->telemkeys.extension().listkeys();
                        message.str(""); message << "fitsinfo[" << i << "] extension userkeys:"; logwrite( function, message.str() );
                        ret = info != nullptr && info->userkeys.extension().listkeys();
                        if ( info != nullptr ) break; // one expbuf is enough
                      }
                      message.str(""); message << "camera_info primary userkeys:"; logwrite( function, message.str() );
                      ret = server.camera_info.userkeys.primary().listkeys();
                      message.str(""); message << "camera_info extension userkeys:"; logwrite( function, message.str() );
                      ret = server.camera_info.userkeys.extension().listkeys();
                      message.str(""); message << "camera_info primary telemkeys:"; logwrite( function, message.str() );
                      ret = server.camera_info.telemkeys.primary().listkeys();
                      message.str(""); message << "camera_info extension telemkeys:"; logwrite( function, message.str() );
                      ret = server.camera_info.telemkeys.extension().listkeys();

                      for ( auto &[chan, keydb] : server.camera_info.systemkeys.elmomap() ) {
                        message.str(""); message << "camera_info elmo extension systemkeys chan " << chan; logwrite( function, message.str() );
                        keydb.listkeys();
                      }

                      for ( auto &[chan, keydb] : server.camera_info.telemkeys.elmomap() ) {
                        message.str(""); message << "camera_info elmo extension telemkeys chan " << chan; logwrite( function, message.str() );
                        keydb.listkeys();
                      }
                    }
                    else {
                      ret = server.camera_info.userkeys.primary().addkey(args);
//                    server.fitsinfo[0]->telemkeys.primary().addkey( "FOO", true, "test telemetry primary key" );
//                    for ( const auto &info : server.fitsinfo ) ret = info->userkeys.extension().addkey( args );
//                    if ( ret != NO_ERROR ) server.camera.log_error( function, "bad syntax" );
                    }
                    }
    else
    if ( cmd == CAMERAD_NATIVE ) {
      logwrite(function, " CALLING NATIVE");
                    ret = server.native(args, retstring);  // @todo make this work with Archon
                    }
#ifdef ASTROCAM
    else
    if ( cmd == CAMERAD_MODEXPTIME ) {
                    ret = server.modify_exptime(args, retstring);
                    }
    else
    if ( cmd == CAMERAD_PAUSE ) {
                    ret = server.native( "PEX" );
                    }
    else
    if ( cmd == CAMERAD_RESUME ) {
                    ret = server.native( "REX" );
                    }
    else
    if ( cmd == CAMERAD_ABORT ) {
                    server.camera_info.abortexposure=true;
                    server.camera.abort();
                    server.abort();
                    ret = 0;
                    }
    else
    if ( cmd == CAMERAD_ISOPEN ) {
                    ret = server.is_connected( retstring );
                    }
    else
    if ( cmd == CAMERAD_USEFRAMES ) {
                    ret = server.access_useframes(args);
                    }
    else
    if ( cmd == CAMERAD_FRAMETRANSFER ) {
                    ret = server.frame_transfer_mode(args, retstring);
                    }
    else
    if ( cmd == CAMERAD_GEOMETRY ) {
                    ret = server.geometry(args, retstring);
                    }
    else
    if ( cmd == CAMERAD_BUFFER ) {
                    ret = server.buffer(args, retstring);
                    }
    else
    if ( cmd == CAMERAD_IMSIZE ) {
                    ret = server._image_size(args, retstring);
                    }
    else
    if ( cmd == CAMERAD_READOUT ) {
                    ret = server.readout(args, retstring);
                    }
    else
    if ( cmd == CAMERAD_BOI ) {
                    ret = server.band_of_interest(args, retstring);
                    }
#endif
#ifdef STA_ARCHON
    else
    if ( cmd == "roi" ) {
                    ret = server.region_of_interest( args, retstring );
                    if (!retstring.empty()) { sock.Write(retstring); sock.Write(" "); }
                    }
    else
    if ( cmd == "isloaded" ) {
                    retstring = server.firmwareloaded ? "true" : "false";
                    sock.Write(retstring);
                    sock.Write(" ");
                    ret = NO_ERROR;
                    }
    else
    if ( cmd == "mode" ) {
                    if (args.empty()) {     // no argument means asking for current mode
                      if (server.modeselected) {
                        ret=NO_ERROR;
                        sock.Write(server.camera_info.current_observing_mode); sock.Write(" ");
                      }
                      else ret=ERROR;       // no mode selected returns an error
                    }
                    else ret = server.set_camera_mode(args);
                    }
    else
    if ( cmd == "getp" ) {
                    ret = server.get_parameter(args, retstring);
                    if (!retstring.empty()) { sock.Write(retstring); sock.Write(" "); }
                    }
    else
    if ( cmd == "setp" ) {
                    ret = server.set_parameter(args);
                    }
    else
    if ( cmd == "loadtiming" ) {
                    if (args.empty()) ret = server.load_timing(retstring);
                    else              ret = server.load_timing(args, retstring);
                    if (!retstring.empty()) { sock.Write(retstring); sock.Write(" "); }
                    }
    else
    if ( cmd == "inreg" ) {
                    ret = server.inreg(args);
                    }
    else
    if ( cmd == "printstatus" ) {
                    ret = server.get_frame_status();
                    if (ret==NO_ERROR) server.print_frame_status();
                    }
    else
    if ( cmd == "readframe" ) {
                    ret = server.read_frame();
                    }
    else
    if ( cmd == "writeframe" ) {
                    ret = server.write_frame();
                    }
    else
    if ( cmd == "cds" ) {
                    ret = server.cds(args, retstring);
                    if (!retstring.empty()) { sock.Write(retstring); sock.Write(" "); }
                    }
    else
    if ( cmd == "heater" ) {
                    ret = server.heater(args, retstring);
                    if (!retstring.empty()) { sock.Write(retstring); sock.Write(" "); }
                    }
    else
    if ( cmd == "sensor" ) {
                    ret = server.sensor(args, retstring);
                    if (!retstring.empty()) { sock.Write(retstring); sock.Write(" "); }
                    }
    else
    if ( cmd == "longexposure" ) {
                    ret = server.longexposure(args, retstring);
                    if (!retstring.empty()) { sock.Write(retstring); sock.Write(" "); }
                    }
    else
    if ( cmd == "hdrshift" ) {
                    ret = server.hdrshift(args, retstring);
                    if (!retstring.empty()) { sock.Write(retstring); sock.Write(" "); }
                    }
    else
    if ( cmd == "trigin" ) {
                    ret = server.trigin(args);
                    }
#endif
    else
    if ( cmd == CAMERAD_EXPOSE ) {
                    ret = server.expose(args);
                    }
    else
    if ( cmd == CAMERAD_EXPTIME ) {
                    // Neither controller allows fractional exposure times
                    // so catch that here.
                    //
                    if ( args.find(".") != std::string::npos ) {
                      ret = ERROR;
                      logwrite(function, "ERROR: fractional exposure times not allowed");
                      // empty the args string so that a call to exptime returns the current exptime
                      //
                      args="";
                      server.exptime(args, retstring);
                    }
                    else { ret = server.exptime(args, retstring); }
                    if (!retstring.empty()) { sock.Write(retstring); sock.Write(" "); }
                    }
    else
    if ( cmd == CAMERAD_BIAS ) {
                    ret = server.bias(args, retstring);
                    if (!retstring.empty()) { sock.Write(retstring); sock.Write(" "); }
                    }
    else
    if ( cmd == CAMERAD_BIN ) {
                    ret = server.bin( args, retstring );
                    if (!retstring.empty()) { sock.Write(retstring); sock.Write(" "); }
                    }
    else
    if ( cmd == CAMERAD_ECHO ) {
                    sock.Write(args);
                    sock.Write("\n");
                    }
    else
    if ( cmd == CAMERAD_INTERFACE ) {
                    ret = server.interface(retstring);
                    sock.Write(retstring);
                    sock.Write(" ");
                    }
    else
    if ( cmd == CAMERAD_TEST ) {
                    ret = server.test(args, retstring);
                    if (!retstring.empty()) { sock.Write(retstring); sock.Write(" "); }
                    }
    // Unknown commands generate an error
    //
    else {
      message.str(""); message << "ERROR: unknown command: " << cmd;
      server.camera.async.enqueue_and_log( function, message.str() );
      ret = ERROR;
    }

    // If retstring not empty then append "DONE" or "ERROR" depending on value of ret,
    // and log the reply along with the command number. Write the reply back to the socket.
    //
    // Don't append anything nor log the reply if the command was just requesting help.
    //
    if (ret != NOTHING) {
      if ( ! retstring.empty() ) retstring.append( " " );
      if ( ret != HELP && ret != JSON ) retstring.append( ret == NO_ERROR ? "DONE" : "ERROR" );

      if ( ret == JSON ) {
        message.str(""); message << "command (" << server.cmd_num << ") reply with JSON message";
        logwrite( function, message.str() );
      }
      else
      if ( ! retstring.empty() && ret != HELP ) {
        retstring.append( "\n" );
        message.str(""); message << "command (" << server.cmd_num << ") reply: " << retstring;
        logwrite( function, message.str() );
      }

      if ( sock.Write( retstring ) < 0 ) connection_open=false;
    }

    if (!sock.isblocking()) break;       // Non-blocking connection exits immediately.
                                         // Keep blocking connection open for interactive session.
  }

  return;
}
/***** Server::doit *********************************************************/

