/**
 * @file    messaged.cpp
 * @brief   this is the main message daemon
 * @details The message daemon is a broker for handling messages between
 *          daemons. All daemons publish and subscribe to this broker,
 *          which ensures the messages are distributed to each subscriber.
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#include "daemonize.h"
#include "logentry.h"
#include "common.h"
#include "utilities.h"
#include <zmqpp/zmqpp.hpp>
#include <zmq.h>
#include <iostream>
#include <csignal>
#include <atomic>
#include <thread>

constexpr char DAEMON_NAME[] = "messaged";

int  main( int argc, char** argv );
void signal_handler( int signo );
void runbroker();
void dothread_runproxy( void* pxsub, void* pxpub );

std::atomic<bool> is_broker_running(false);

/***** main *******************************************************************/
/**
 * @brief      the main function
 * @param[in]  int argc
 * @param[in]  char** argv
 * @return     0
 *
 */
int main( int argc, char** argv ) {
  bool start_daemon = true;  // daemonize by default, override with command line arg

  // allow running in the foreground
  //
  if ( cmdOptionExists( argv, argv+argc, "--foreground" ) ) {
    start_daemon = false;
  }

  const std::string daemon_stdout="/dev/null";
  const std::string daemon_stderr="/tmp/"+std::string(DAEMON_NAME)+".stderr";

  // daemonize, close fds
  //
  if ( start_daemon ) {
    logwrite( "messaged::main", "starting daemon" );
    Daemon::daemonize( DAEMON_NAME, "/tmp", daemon_stdout, daemon_stderr, "", true );
  }

  logwrite( "messaged::main", "daemonized. child process is running" );

  // handle signals for clean shutdown
  //
  signal( SIGTERM, [](int) { std::exit(0); } );
  signal( SIGINT,  [](int) { std::exit(0); } );

  // now the child proc runs the actual broker
  //
  runbroker();

  return 0;
}


void signal_handler( int signo ) {
  logwrite( "messaged::signal_handler", "received signal: "+std::to_string(signo) );
  is_broker_running.store(false);
}


void runbroker() {
  // initialize the ZeroMQ context
  //
  void* context = zmq_ctx_new();

  // create the XSUB and XPUB sockets using libzmq
  //
  void* xsub_socket = zmq_socket(context, ZMQ_XSUB);
  void* xpub_socket = zmq_socket(context, ZMQ_XPUB);

  // bind the sockets
  //
  zmq_bind(xsub_socket, "tcp://127.0.0.1:5555");
  zmq_bind(xpub_socket, "tcp://127.0.0.1:5556");

  // start the proxy in a separate thread because zmq_proxy blocks
  //
  logwrite( "messaged::runbroker", "starting message broker" );
  std::thread proxy_thread(dothread_runproxy, xsub_socket, xpub_socket);

  {
  BoolState broker_running( is_broker_running );

  while ( is_broker_running.load() ) {
    std::this_thread::sleep_for( std::chrono::milliseconds(100) );
  }
  }

  proxy_thread.join();

  // close the sockets
  //
  zmq_close(xsub_socket);
  zmq_close(xpub_socket);

  // clean up the context
  //
  zmq_ctx_destroy(context);
}

void dothread_runproxy( void* pxsub, void* pxpub ) {
  if (zmq_proxy(pxsub, pxpub, nullptr) == -1) {
    logwrite( "messaged::dothread_runproxy", "ERROR proxy failed: "+std::string(zmq_strerror(zmq_errno())) );
  }
}
