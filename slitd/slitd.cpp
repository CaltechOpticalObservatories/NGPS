/**
 * @file    slitd.cpp
 * @brief   this is the main slit daemon
 * @details 
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#include "build_date.h"
#include "slitd.h"
#include "daemonize.h"

Slit::Server slitd;
std::string logpath; 

/** signal_handler ***********************************************************/
/**
 * @fn     signal_handler
 * @brief  handles ctrl-C
 * @param  int signo
 * @return nothing
 *
 */
void signal_handler(int signo) {
  std::string function = "Slit::signal_handler";
  switch (signo) {
    case SIGTERM:
    case SIGINT:
      logwrite(function, "received termination signal");
//    slitd.common.message.enqueue("exit");      // shutdown the async_main thread if running
      slitd.exit_cleanly();                      // shutdown the daemon
      break;
    case SIGHUP:
      logwrite(function, "caught SIGHUP");
      slitd.configure_slitd();                   // TODO can (/should) this be done while running?
      break;
    case SIGPIPE:
      logwrite(function, "caught SIGPIPE");
      break;
    default:
      logwrite(function, "received unknown signal");
//    slitd.common.message.enqueue("exit");      // shutdown the async_main thread if running
      slitd.exit_cleanly();                      // shutdown the daemon
      break;
  }
  return;
}
/** signal_handler ***********************************************************/


int  main(int argc, char **argv);           // main thread (just gets things started)
void new_log_day( );                        // create a new log each day
void block_main(Network::TcpSocket sock);   // this thread handles requests on blocking port
void thread_main(Network::TcpSocket sock);  // this thread handles requests on non-blocking port
void async_main(Network::UdpSocket sock);   // this thread handles the asyncrhonous UDP message port
void doit(Network::TcpSocket sock);         // the worker thread


/** main *********************************************************************/
/**
 * @fn     main
 * @brief  the main function
 * @param  int argc, char** argv
 * @return 0
 *
 */
int main(int argc, char **argv) {
  std::string function = "Slit::main";
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
      slitd.config.filename = std::string( filename );
    }
  }
  else

  // if no "-f <filename>" then as long as there's at least one arg,
  // assume that is the config file name.
  //
  if (argc>1) {
    slitd.config.filename = std::string( argv[1] );
  }
  else {
    logwrite(function, "ERROR: no configuration file specified");
    slitd.exit_cleanly();
  }

  if ( slitd.config.read_config(slitd.config) != NO_ERROR) {          // read configuration file specified on command line
    logwrite(function, "ERROR: unable to configure system");
    slitd.exit_cleanly();
  }

  for (int entry=0; entry < slitd.config.n_entries; entry++) {
    if (slitd.config.param[entry] == "LOGPATH") logpath = slitd.config.arg[entry];
    if (slitd.config.param[entry] == "TM_ZONE") zone = slitd.config.arg[entry];
    if (slitd.config.param[entry] == "DAEMON")  daemon_in = slitd.config.arg[entry];
  }
  if (logpath.empty()) {
    logwrite(function, "ERROR: LOGPATH not specified in configuration file");
    slitd.exit_cleanly();
  }

  if ( !daemon_in.empty() && daemon_in == "yes" ) start_daemon = true;
  else
  if ( !daemon_in.empty() && daemon_in == "no"  ) start_daemon = false;
  else {
    message.str(""); message << "ERROR: unrecognized argument DAEMON=" << daemon_in << ", expected { yes | no }";
    logwrite( function, message.str() );
    slitd.exit_cleanly();
  }

  // check for "-d" command line option last so that the command line
  // can override the config file to start as daemon
  //
  if ( cmdOptionExists( argv, argv+argc, "-d" ) ) {
    start_daemon = true;
  }

  if ( start_daemon ) {
    logwrite( function, "starting daemon" );
    Daemon::daemonize( "slitd", "/tmp", "", "", "" );
  }

  if ( (init_log(logpath) != 0) ) {                      // initialize the logging system
    logwrite(function, "ERROR: unable to initialize logging system");
    slitd.exit_cleanly();
  }

  message << "this version built " << BUILD_DATE << " " << BUILD_TIME;
  logwrite(function, message.str());

  message.str(""); message << slitd.config.n_entries << " lines read from " << slitd.config.filename;
  logwrite(function, message.str());

  if (ret==NO_ERROR) ret=slitd.configure_slitd();          // get needed values out of read-in configuration file for the daemon

  if (ret != NO_ERROR) {
    logwrite(function, "ERROR: unable to configure system");
    slitd.exit_cleanly();
  }

  if (slitd.nbport == -1 || slitd.blkport == -1) {
    logwrite(function, "ERROR: slitd ports not configured");
    slitd.exit_cleanly();
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

  Network::TcpSocket s(slitd.blkport, true, -1, 0);  // instantiate TcpSocket object with blocking port
  s.Listen();                                        // create a listening socket
  socklist.push_back(s);                             // add it to the socklist vector
  std::thread(block_main, socklist[0]).detach();     // spawn a thread to handle requests on this socket

  // pre-thread N_THREADS-1 detached threads to handle requests on the non-blocking port
  // thread #0 is reserved for the blocking port (above)
  //
  for (int i=1; i<N_THREADS; i++) {                  // create N_THREADS-1 non-blocking socket objects
    if (i==1) {                                      // first one only
      Network::TcpSocket s(slitd.nbport, false, CONN_TIMEOUT, i);   // instantiate TcpSocket object, non-blocking port, CONN_TIMEOUT timeout
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
  Network::UdpSocket async(slitd.asyncport, slitd.asyncgroup);
  std::thread(async_main, async).detach();

  // thread to start a new logbook each day
  //
  std::thread( new_log_day ).detach();

  for (;;) pause();                                  // main thread suspends
  return 0;
}
/** main *********************************************************************/


/** new_log_day **************************************************************/
/**
 * @fn     new_log_day
 * @brief  creates a new logbook each day
 * @return nothing
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
    init_log( logpath );
  }
}
/** new_log_day **************************************************************/


/** block_main ***************************************************************/
/**
 * @fn     block_main
 * @brief  main function for blocking connection thread
 * @param  Network::TcpSocket sock, socket object
 * @return nothing
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
/** block_main ***************************************************************/


/** thread_main **************************************************************/
/**
 * @fn     thread_main
 * @brief  main function for all non-blocked threads
 * @param  Network::TcpSocket sock, socket object
 * @return nothing
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
    slitd.conn_mutex.lock();
    sock.Accept();
    slitd.conn_mutex.unlock();
    doit(sock);                // call function to do the work
    sock.Close();
  }
  return;
}
/** thread_main **************************************************************/


/** async_main ***************************************************************/
/**
 * @fn     async_main
 * @brief  asynchronous message sending thread
 * @param  Network::UdpSocket sock, socket object
 * @return nothing
 *
 * Loops forever, when a message arrives in the status message queue it is
 * sent out via multi-cast UDP datagram.
 *
 */
void async_main(Network::UdpSocket sock) {
  std::string function = "Slit::async_main";
  int retval;

  logwrite( function, "NOT IMPLEMENTED" );
  return;

  retval = sock.Create();                                   // create the UDP socket
  if (retval < 0) {
    logwrite(function, "error creating UDP multicast socket for asynchronous messages");
    slitd.exit_cleanly();                                   // do not continue on error
  }
  if (retval==1) {                                          // exit this thread but continue with daemon
    logwrite(function, "asyncrhonous message port disabled by request");
  }

/***
  while (1) {
    std::string message = slitd.common.message.dequeue();   // get the latest message from the queue (blocks)
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
***/
  return;
}
/** async_main ***************************************************************/


/** doit *********************************************************************/
/**
 * @fn     doit
 * @brief  the workhorse of each thread connetion
 * @param  int thr
 * @return nothin
 *
 * stays open until closed by client
 *
 * commands come in the form: 
 * <device> [all|<app>] [_BLOCK_] <command> [<arg>]
 *
 */
void doit(Network::TcpSocket sock) {
  std::string function = "Slit::doit";
  char  buf[BUFSIZE+1];
  long  ret;
  std::stringstream message;
  std::string cmd, args;        // arg string is everything after command
  std::vector<std::string> tokens;

  bool connection_open=true;

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
        message.str(""); message << "Read error on fd " << sock.getfd() << ": " << strerror(errno); logwrite(function, message.str());
      }
      if (ret==0) {               // or a timeout
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

      sock.id = ++slitd.cmd_num;
      if ( slitd.cmd_num == INT_MAX ) slitd.cmd_num = 0;

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

    if ( cmd.compare( "exit" ) == 0 ) {
//                  slitd.common.message.enqueue("exit");      // shutdown the async message thread if running
                    slitd.exit_cleanly();                      // shutdown the daemon
    }
    else

    // open
    //
    if ( cmd.compare( SLITD_OPEN ) == 0 ) {
                    ret = slitd.interface.open( );
    }
    else

    // isopen
    //
    if ( cmd.compare( SLITD_ISOPEN ) == 0 ) {
                    bool isopen = slitd.interface.isopen( );
                    if ( isopen ) retstring = "true"; else retstring = "false";
                    ret = NO_ERROR;
    }
    else

    // close
    //
    if ( cmd.compare( "close" ) == 0 ) {
                    ret = slitd.interface.send_command( "close" );  // just needed for the emulator
                    ret = slitd.interface.close();
    }
    else

    if ( cmd.compare( "echo" ) == 0 ) {
                    sock.Write( args );
                    sock.Write( "\n" );
    }
    else

    // home
    //
    if ( cmd.compare( SLITD_HOME ) == 0 ) {
                    ret = slitd.interface.home( );
    }
    else

    // is_home
    //
    if ( cmd.compare( SLITD_ISHOME ) == 0 ) {
                    ret = slitd.interface.is_home( retstring );
    }
    else

    // set width, offset
    //
    if ( cmd.compare( SLITD_SET ) == 0 ) {
                    ret = slitd.interface.set( args, retstring );
    }
    else

    // get width, offset
    //
    if ( cmd.compare( "get" ) == 0 ) {
                    ret = slitd.interface.get( retstring );
    }
    else

    // moveabs
    //
    if ( cmd.compare( "moveabs" ) == 0 ) {
                    ret = slitd.interface.move_abs( args );
    }
    else

    // moverel
    //
    if ( cmd.compare( "moverel" ) == 0 ) {
                    ret = slitd.interface.move_rel( args );
    }
    else

    // stop
    //
    if ( cmd.compare( "stop" ) == 0 ) {
                    ret = slitd.interface.stop( );
    }

    // all other commands go straight to the controller interface
    //
    else {
      try {
        std::transform( sbuf.begin(), sbuf.end(), sbuf.begin(), ::toupper );  // make uppercase
      }
      catch (...) {
        logwrite( function, "error converting command to uppercae" );
        ret=ERROR;
      }
      ret = slitd.interface.send_command( sbuf, retstring );
    }

    if (ret != NOTHING) {
      if ( !retstring.empty() ) retstring.append( " " );
      std::string term=(ret==0?"DONE\n":"ERROR\n");
      retstring.append( term );
      if ( sock.Write( retstring ) < 0 ) connection_open=false;
    }

    if (!sock.isblocking()) break;       // Non-blocking connection exits immediately.
                                         // Keep blocking connection open for interactive session.
  }

  sock.Close();
  return;
}
/** doit *********************************************************************/

