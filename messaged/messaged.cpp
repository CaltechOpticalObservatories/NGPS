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
#include <unordered_map>
#include <string>
#include <utility>
#include <chrono>
#include <cstdint>

constexpr char DAEMON_NAME[] = "messaged";

int  main( int argc, char** argv );
void signal_handler( int signo );
void runbroker();
void dothread_runlvc( void* frontend, void* backend );

std::atomic<bool> is_broker_running(false);

/***** last-value-cache helpers ***********************************************/
namespace {
  // last published message per topic: topic -> { topic_frame, payload_frame }.
  // Touched only by the single LVC thread, so no locking is required.
  std::unordered_map<std::string, std::pair<std::string,std::string>> g_lvc;

  // receive one frame from sock as a std::string (empty on error)
  std::string recv_frame( void* sock ) {
    zmq_msg_t msg;
    zmq_msg_init( &msg );
    int n = zmq_msg_recv( &msg, sock, 0 );
    std::string out;
    if ( n >= 0 ) out.assign( static_cast<char*>( zmq_msg_data(&msg) ), zmq_msg_size(&msg) );
    zmq_msg_close( &msg );
    return out;
  }

  // true if another frame of the current multipart message is pending
  bool has_more( void* sock ) {
    int more = 0;
    size_t sz = sizeof(more);
    zmq_getsockopt( sock, ZMQ_RCVMORE, &more, &sz );
    return more != 0;
  }

  // send one frame to sock
  void send_frame( void* sock, const std::string &s, bool more ) {
    zmq_send( sock, s.data(), s.size(), more ? ZMQ_SNDMORE : 0 );
  }
}
/***** last-value-cache helpers ***********************************************/

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

  // unlimited queues: prevent silent drops when any daemon is slow to drain.
  // LINGER=0: broker exits cleanly without blocking on pending messages.
  //
  int zero = 0;
  zmq_setsockopt( xsub_socket, ZMQ_SNDHWM,  &zero, sizeof(zero) );
  zmq_setsockopt( xsub_socket, ZMQ_RCVHWM,  &zero, sizeof(zero) );
  zmq_setsockopt( xsub_socket, ZMQ_LINGER,  &zero, sizeof(zero) );
  zmq_setsockopt( xpub_socket, ZMQ_SNDHWM,  &zero, sizeof(zero) );
  zmq_setsockopt( xpub_socket, ZMQ_RCVHWM,  &zero, sizeof(zero) );
  zmq_setsockopt( xpub_socket, ZMQ_LINGER,  &zero, sizeof(zero) );

  // XPUB_VERBOSE: deliver EVERY subscribe message to the broker (not just the
  // first per topic). Required so the LVC can replay the last value to each new
  // subscriber, including a restarted daemon re-subscribing to a topic that
  // another daemon is already subscribed to.
  //
  int one = 1;
  zmq_setsockopt( xpub_socket, ZMQ_XPUB_VERBOSE, &one, sizeof(one) );

  // bind the sockets
  //
  zmq_bind(xsub_socket, "tcp://127.0.0.1:5555");
  zmq_bind(xpub_socket, "tcp://127.0.0.1:5556");

  // start the last-value-cache broker in a separate thread (it blocks)
  //
  logwrite( "messaged::runbroker", "starting last-value-cache message broker" );
  std::thread proxy_thread(dothread_runlvc, xsub_socket, xpub_socket);

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

/***** dothread_runlvc ********************************************************/
/**
 * @brief      last-value-cache broker loop (replaces zmq_proxy)
 * @details    Forwards publisher traffic (frontend XSUB) to subscribers
 *             (backend XPUB) exactly like a plain proxy, but additionally:
 *               (1) caches the last message seen on each topic, and
 *               (2) replays that cached message to any newly subscribing
 *                   socket, so a restarted daemon immediately receives the
 *                   current state of every provider without a forced snapshot.
 *             A subscribe-all is re-asserted on the frontend on a fixed tick so
 *             every (re)connected publisher keeps forwarding to the broker,
 *             without relying on XSUB replay-on-attach semantics.
 * @param[in]  frontend  XSUB socket (publishers connect here, port 5555)
 * @param[in]  backend   XPUB socket (subscribers connect here, port 5556)
 *
 */
void dothread_runlvc( void* frontend, void* backend ) {
  zmq_pollitem_t items[2] = {
    { frontend, 0, ZMQ_POLLIN, 0 },   // publisher traffic
    { backend,  0, ZMQ_POLLIN, 0 },   // subscription traffic
  };

  const uint8_t subscribe_all = 0x01;                       // 0x01 = subscribe, empty prefix = all topics
  const auto    resub_period  = std::chrono::seconds(2);
  auto          last_resub    = std::chrono::steady_clock::now() - resub_period;  // force immediate first send

  logwrite( "messaged::dothread_runlvc", "last-value-cache broker running" );

  while ( is_broker_running.load() ) {
    if ( zmq_poll( items, 2, 200 ) < 0 ) {                  // 200ms tick so we can observe the stop flag
      if ( zmq_errno() == EINTR ) continue;
      logwrite( "messaged::dothread_runlvc", "ERROR poll failed: "+std::string(zmq_strerror(zmq_errno())) );
      break;
    }

    // ---- publisher -> cache last value per topic, then fan out to subscribers
    //
    if ( items[0].revents & ZMQ_POLLIN ) {
      std::string topic   = recv_frame( frontend );
      std::string payload = has_more( frontend ) ? recv_frame( frontend ) : std::string();
      while ( has_more( frontend ) ) recv_frame( frontend );   // drain any unexpected extra frames

      // Cache normal telemetry topics; skip control topics (e.g. "_snapshot")
      // so a new subscriber is never handed a replayed stale request.
      //
      if ( !topic.empty() && topic.front() != '_' ) {
        g_lvc[topic] = { topic, payload };
      }

      send_frame( backend, topic,   true  );
      send_frame( backend, payload, false );
    }

    // ---- new subscription -> replay that topic's last value to the subscriber
    //
    if ( items[1].revents & ZMQ_POLLIN ) {
      std::string sub = recv_frame( backend );               // [0x01|0x00][topic...]
      while ( has_more( backend ) ) recv_frame( backend );   // subscriptions are single-frame; drain defensively

      if ( !sub.empty() && static_cast<uint8_t>( sub.front() ) == 0x01 ) {   // 0x01 == subscribe
        auto it = g_lvc.find( sub.substr(1) );
        if ( it != g_lvc.end() ) {
          logwrite( "messaged::dothread_runlvc",
                    "[DEBUG] replaying cached topic "+it->second.first
                    +" ("+std::to_string( it->second.second.size() )+" bytes) to new subscriber" );
          send_frame( backend, it->second.first,  true  );
          send_frame( backend, it->second.second, false );
        }
      }
      // unsubscribe (0x00) is ignored: the cache persists so future subscribers
      // still receive the last value.
    }

    // ---- keep every (re)connected publisher forwarding to the broker
    //
    auto now = std::chrono::steady_clock::now();
    if ( now - last_resub >= resub_period ) {
      zmq_send( frontend, &subscribe_all, 1, 0 );
      last_resub = now;
    }
  }

  logwrite( "messaged::dothread_runlvc", "last-value-cache broker stopped" );
}
/***** dothread_runlvc ********************************************************/
