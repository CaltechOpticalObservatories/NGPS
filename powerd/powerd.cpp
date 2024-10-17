/**
 * @file    powerd.cpp
 * @brief   this is the main power daemon
 * @details 
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#include "build_date.h"
#include "powerd.h"
#include "daemonize.h"

Power::Server powerd;
std::string logpath; 

/***** signal_handler *********************************************************/
/**
 * @fn         signal_handler
 * @brief      handles ctrl-C
 * @param[in]  int signo
 * @return     nothing
 *
 */
void signal_handler(int signo) {
  std::string function = "Power::signal_handler";
  std::stringstream message;

  switch (signo) {
    case SIGTERM:
    case SIGINT:
      logwrite(function, "received termination signal");
      message << "NOTICE:" << Power::DAEMON_NAME << " exit";
      powerd.interface.async.enqueue( message.str() );
      powerd.exit_cleanly();                     // shutdown the daemon
      break;
    case SIGHUP:
      logwrite(function, "caught SIGHUP");
      powerd.configure_powerd();                 // TODO can (/should) this be done while running?
      break;
    case SIGPIPE:
      logwrite(function, "caught SIGPIPE");
      break;
    default:
      message << "received unknown signal " << strsignal(signo);
      logwrite( function, message.str() );
      message.str(""); message << "NOTICE:" << Power::DAEMON_NAME << " exit";
      powerd.interface.async.enqueue( message.str() );
      powerd.exit_cleanly();                     // shutdown the daemon
      break;
  }
  return;
}
/***** signal_handler *********************************************************/


int  main(int argc, char **argv);           ///< main thread (just gets things started)
void new_log_day( );                        ///< create a new log each day
void block_main(Network::TcpSocket sock);   ///< this thread handles requests on blocking port
void thread_main(Network::TcpSocket sock);  ///< this thread handles requests on non-blocking port
void async_main(Network::UdpSocket sock);   ///< this thread handles the asyncrhonous UDP message port
void doit(Network::TcpSocket sock);         ///< the worker thread


/***** main *******************************************************************/
/**
 * @fn         main
 * @brief      the main function
 * @param[in]  int argc, char** argv
 * @return     0
 *
 */
int main(int argc, char **argv) {
  std::string function = "Power::main";
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
      powerd.config.filename = std::string( filename );
    }
  }
  else

  // if no "-f <filename>" then as long as there's at least one arg,
  // assume that is the config file name.
  //
  if (argc>1) {
    powerd.config.filename = std::string( argv[1] );
  }
  else {
    logwrite(function, "ERROR: no configuration file specified");
    powerd.exit_cleanly();
  }

  if ( powerd.config.read_config(powerd.config) != NO_ERROR) {        // read configuration file specified on command line
    logwrite(function, "ERROR: unable to configure system");
    powerd.exit_cleanly();
  }

  for (int entry=0; entry < powerd.config.n_entries; entry++) {
    if (powerd.config.param[entry] == "LOGPATH") logpath = powerd.config.arg[entry];
    if (powerd.config.param[entry] == "DAEMON")  daemon_in = powerd.config.arg[entry];

    if (powerd.config.param[entry] == "TM_ZONE") {
      if ( powerd.config.arg[entry] != "UTC" && powerd.config.arg[entry] != "local" ) {
        message.str(""); message << "ERROR invalid TM_ZONE=" << powerd.config.arg[entry] << ": expected UTC|local";
        logwrite( function, message.str() );
        powerd.exit_cleanly();
      }
      tmzone_cfg = powerd.config.arg[entry];
      message.str(""); message << "config:" << powerd.config.param[entry] << "=" << powerd.config.arg[entry];
      logwrite( function, message.str() );
    }

  }

  if (logpath.empty()) {
    logwrite(function, "ERROR: LOGPATH not specified in configuration file");
    powerd.exit_cleanly();
  }

  if ( !daemon_in.empty() && daemon_in == "yes" ) start_daemon = true;
  else
  if ( !daemon_in.empty() && daemon_in == "no"  ) start_daemon = false;
  else {
    message.str(""); message << "ERROR: unrecognized argument DAEMON=" << daemon_in << ", expected { yes | no }";
    logwrite( function, message.str() );
    powerd.exit_cleanly();
  }

  // check for "-d" command line option last so that the command line
  // can override the config file to start as daemon
  //
  if ( cmdOptionExists( argv, argv+argc, "-d" ) ) {
    start_daemon = true;
  }

  if ( start_daemon ) {
    logwrite( function, "starting daemon" );
    Daemon::daemonize( Power::DAEMON_NAME, "/tmp", "", "", "" );
  }

  if ( ( init_log( logpath, Power::DAEMON_NAME ) != 0 ) ) {          // initialize the logging system
    logwrite(function, "ERROR: unable to initialize logging system");
    powerd.exit_cleanly();
  }

  message << "this version built " << BUILD_DATE << " " << BUILD_TIME;
  logwrite(function, message.str());

  message.str(""); message << powerd.config.n_entries << " lines read from " << powerd.config.filename;
  logwrite(function, message.str());

  if (ret==NO_ERROR) ret=powerd.configure_powerd();        // get needed values out of read-in configuration file for the daemon

  if (ret != NO_ERROR) {
    logwrite(function, "ERROR: unable to configure system");
    powerd.exit_cleanly();
  }

  if (powerd.nbport == -1 || powerd.blkport == -1) {
    logwrite(function, "ERROR: powerd ports not configured");
    powerd.exit_cleanly();
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

  Network::TcpSocket s(powerd.blkport, true, -1, 0); // instantiate TcpSocket object with blocking port
  s.Listen();                                        // create a listening socket
  socklist.push_back(s);                             // add it to the socklist vector
  std::thread(block_main, socklist[0]).detach();     // spawn a thread to handle requests on this socket

  // pre-thread N_THREADS-1 detached threads to handle requests on the non-blocking port
  // thread #0 is reserved for the blocking port (above)
  //
  for (int i=1; i<N_THREADS; i++) {                  // create N_THREADS-1 non-blocking socket objects
    if (i==1) {                                      // first one only
      Network::TcpSocket s(powerd.nbport, false, CONN_TIMEOUT, i);   // instantiate TcpSocket object, non-blocking port, CONN_TIMEOUT timeout
      s.Listen();                                    // create a listening socket
      socklist.push_back(s);
    }
    else {                                           // subsequent socket objects are copies of the first
      Network::TcpSocket s = socklist[1];            // copy the first one, which has a valid listening socket
      s.id = i;
      socklist.push_back(s);
    }
    std::thread(thread_main, socklist[i]).detach();  // spawn a thread to handle each non-blocking socket request
  }

  // Instantiate a multicast UDP object and spawn a thread to send asynchronous messages
  //
  Network::UdpSocket async(powerd.asyncport, powerd.asyncgroup);
  std::thread(async_main, async).detach();

  // thread to start a new logbook each day
  //
  std::thread( new_log_day ).detach();

  // Open a connection to the hardware if set in the config file.
  // Default is yes but this can be overridden (set to no) in the
  // config file, which will require the user to send an "open" command.
  //
  if ( powerd.open_on_start ) powerd.interface.open();

  for (;;) pause();                                  // main thread suspends
  return 0;
}
/***** main *******************************************************************/


/***** new_log_day ************************************************************/
/**
 * @fn         new_log_day
 * @brief      creates a new logbook each day
 * @param[in]  none
 * @return     nothing
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
    init_log( logpath, Power::DAEMON_NAME );
  }
}
/***** new_log_day ************************************************************/


/***** block_main *************************************************************/
/**
 * @fn         block_main
 * @brief      main function for blocking connection thread
 * @param[in]  Network::TcpSocket sock, socket object
 * @return     nothing
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


/***** thread_main ************************************************************/
/**
 * @fn         thread_main
 * @brief      main function for all non-blocked threads
 * @param[in]  Network::TcpSocket sock, socket object
 * @return     nothing
 *
 * accepts a socket connection and processes the request by
 * calling function doit()
 *
 * There are N_THREADS-1 of these, one for each non-blocking connection.
 * These threads never terminate.
 *
 * This function differs from block_main only in that the call to Accept
 * is mutex-protected.
 *
 */
void thread_main(Network::TcpSocket sock) {
  while (1) {
    powerd.conn_mutex.lock();
    sock.Accept();
    powerd.conn_mutex.unlock();
    doit(sock);                // call function to do the work
    sock.Close();
  }
  return;
}
/***** thread_main ************************************************************/


/***** async_main *************************************************************/
/**
 * @fn         async_main
 * @brief      asynchronous message sending thread
 * @param[in]  Network::UdpSocket sock, socket object
 * @return     nothing
 *
 * Loops forever, when a message arrives in the status message queue it is
 * sent out via multi-cast UDP datagram.
 *
 */
void async_main(Network::UdpSocket sock) {
  std::string function = "Power::async_main";
  int retval;

  retval = sock.Create();                                   // create the UDP socket
  if (retval < 0) {
    logwrite(function, "error creating UDP multicast socket for asynchronous messages");
    powerd.exit_cleanly();                                  // do not continue on error
  }
  if (retval==1) {                                          // exit this thread but continue with daemon
    logwrite(function, "asyncrhonous message port disabled by request");
  }

  while (1) {
    std::string message = powerd.interface.async.dequeue(); // get the latest message from the queue (blocks)
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
/***** async_main *************************************************************/


/***** doit *******************************************************************/
/**
 * @fn         doit
 * @brief      the workhorse of each thread connetion
 * @param[in]  int thr
 * @return     nothing
 *
 * stays open until closed by client
 *
 * commands come in the form: 
 * <device> [all|<app>] [_BLOCK_] <command> [<arg>]
 *
 */
void doit(Network::TcpSocket sock) {
  std::string function = "Power::doit";
  char  buf[BUFSIZE+1];
  long  ret;
  std::stringstream message;
  std::string cmd, args;        // arg string is everything after command
  std::vector<std::string> tokens;

  bool connection_open=true;

  message.str(""); message << "accepted connection on fd " << sock.getfd();
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
    std::string sbuf = buf;
    char delim='\n';
    if ( ( ret=sock.Read( sbuf, delim ) ) <= 0 ) {
      if (ret<0) {                // could be an actual read error
        message.str(""); message << "Read error on fd " << sock.getfd() << ": " << strerror(errno);
        logwrite(function, message.str());
      }
      if (ret==-2) {              // or a timeout
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

    if (sbuf.empty()) {sock.Write("\n"); continue;}  // acknowledge empty command so client doesn't time out

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

      sock.id = ++powerd.cmd_num;
      if ( powerd.cmd_num == INT_MAX ) powerd.cmd_num = 0;

      message.str(""); message << "received command on fd " << sock.getfd() << " (" << sock.id << "): " << cmd << " " << args;
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
    std::string retstring="";

    if ( ! powerd.interface.missing.empty() ) { // notify someone!
      message.str(""); message << "NOTICE:" << powerd.interface.missing;
      powerd.interface.async.enqueue( message.str() );
    }

    if ( cmd.compare( "help" ) == 0 || cmd.compare( "?" ) == 0 ) {
                    for ( auto s : POWERD_SYNTAX ) { retstring.append( s ); retstring.append( "\n" ); }
                    ret = NO_ERROR;
    }
    else

    if ( cmd.compare( "exit" )==0 ) {
//                  powerd.common.message.enqueue("exit");     // shutdown the async message thread if running
                    powerd.exit_cleanly();                     // shutdown the daemon
    }
    else

    // open
    //
    if ( cmd.compare( POWERD_OPEN ) == 0 ) {
                    ret = powerd.interface.open( );
                    retstring = powerd.interface.missing;
    }
    else

    // close
    //
    if ( cmd.compare( POWERD_CLOSE ) == 0 ) {
                    ret = powerd.interface.close( );
    }
    else

    // reopen
    //
    if ( cmd.compare( POWERD_REOPEN ) == 0 ) {
                    if ( args=="?" ) { sock.Write( POWERD_REOPEN+"\n  closes, reads cfg file, then re-opens connections to hardware\n" ); }
                    else {
                      ret  = powerd.interface.close();
                      usleep( 100000 );
                      ret |= powerd.configure_powerd();
                      ret |= powerd.interface.open();
                      retstring = powerd.interface.missing;
                    }
    }
    else

    // list devices
    //
    if ( cmd.compare( POWERD_LIST ) == 0 ) {
                    powerd.interface.list( args, retstring );
                    retstring.append( " DONE\n" );
                    if ( sock.Write( retstring ) < 0 ) connection_open=false;
    }
    else

    // power status
    //
    if ( cmd.compare( POWERD_STATUS ) == 0 ) {
                    ret = powerd.interface.status( args, retstring );
                    if ( ret==NO_ERROR ) {
                      ret=NOTHING;
                      if ( sock.Write( retstring ) < 0 ) connection_open=false;
                    }
    }
    else

    // isopen
    //
    if ( cmd.compare( POWERD_ISOPEN ) == 0 ) {
                    bool isopen = powerd.interface.isopen( );
                    if ( isopen ) retstring = "true"; else retstring = "false";
                    ret = NO_ERROR;
    }
    else

    // send telemetry upon request
    //
    if ( cmd == POWERD_TELEMREQUEST ) {
                    if ( args=="?" || args=="help" ) {
                      retstring=POWERD_TELEMREQUEST+"\n";
                      retstring.append( "  Returns a serialized JSON message containing telemetry\n" );
                      retstring.append( "  information, terminated with \"EOF\\n\".\n" );
                      ret=HELP;
                    }
                    else {
                      powerd.interface.make_telemetry_message( retstring );
                      ret = JSON;
                    }
    }

    // all other commands go to the powerd interface for parsing
    //
    else {
                    try {
                      std::transform( sbuf.begin(), sbuf.end(), sbuf.begin(), ::toupper );  // make uppercase
                    }
                    catch( ...) {
                      logwrite( function, "ERROR converting command to uppercase" );
                      ret = ERROR;
                    }
                    ret = powerd.interface.command( sbuf, retstring );                      // send the command
    }

    if (ret != NOTHING) {
      if ( ! retstring.empty() ) retstring.append( " " );
      if ( ret != HELP && ret != JSON ) retstring.append( ret == 0 ? "DONE" : "ERROR" );

      if ( ret == JSON ) {
        message.str(""); message << "command (" << powerd.cmd_num << ") reply with JSON message";
        logwrite( function, message.str() );
      }
      else
      if ( ! retstring.empty() && ret != HELP
                               && cmd != POWERD_STATUS && cmd != POWERD_LIST ) {
        retstring.append( "\n" );
        message.str(""); message << "command (" << powerd.cmd_num << ") reply: " << retstring;
        logwrite( function, message.str() );
      }

      if ( sock.Write( retstring ) < 0 ) connection_open=false;
    }

    if (!sock.isblocking()) break;       // Non-blocking connection exits immediately.
                                         // Keep blocking connection open for interactive session.
  }

  return;
}
/***** doit *******************************************************************/

