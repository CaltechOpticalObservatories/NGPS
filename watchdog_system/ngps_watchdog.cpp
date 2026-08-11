/**
 * @file    ngps_watchdog.cpp
 * @brief   external watchdog that restarts hung (alive-but-unresponsive) NGPS daemons
 * @details systemd's Restart=always (in ngps@.service) already restarts a daemon
 *          that exits. It cannot detect a daemon that is alive but frozen, because
 *          nothing exits. This program fills that gap with black-box liveness
 *          probes, restarting (via systemctl) any daemon that systemd reports
 *          active but that fails to answer for several consecutive probes.
 *
 *          Command-serving daemons are probed over their TCP command port with
 *          the side-effect-free "ping" command, expecting "pong". The messaged
 *          broker has no command port, so it is probed functionally: a ZeroMQ
 *          message is published to the broker and must round-trip back through it
 *          within the timeout (this tests the broker's actual forwarding job).
 *
 *          This process is itself watched by systemd via WatchdogSec= in
 *          ngps-watchdog.service. It emits sd_notify("WATCHDOG=1") from inside the
 *          probe loop (tied to real progress), so a wedged loop is restarted by
 *          systemd, terminating the "who watches the watchman" chain at PID 1.
 *
 *          Aside from ZeroMQ (zmqpp/zmq, needed only to probe the broker the same
 *          way clients do), this tool is standalone: it depends on libc, the C++
 *          standard library, and the header-only sd_notify.h and common_commands.h.
 *          It does not link the daemon/common infrastructure (logging, config,
 *          network classes). Logging is to stderr (systemd journal/syslog).
 *
 *          Ports and broker endpoints are read from the sequencer's config file,
 *          the single source of truth already shared with the sequencer.
 * @author  David Hale <dhale@caltech.edu>
 *
 */

#include "common_commands.h"       // CMD_PING / CMD_PONG (header-only: <string>)
#include "sd_notify.h"             // header-only; no daemon logging/config/network coupling
#include "message_keys.h"          // header-only Topic::/Key::/Daemon:: constants -- no daemon-infra coupling

#include <json.hpp>                // nlohmann::json, for the Topic::SNAPSHOT request payload

#include <zmqpp/zmqpp.hpp>

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <thread>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

namespace {
  constexpr int  PROBE_PERIOD_SEC  = 10;    ///< seconds between probe rounds
  constexpr int  PROBE_TIMEOUT_MS  = 5000;  ///< time to wait for a probe reply
  constexpr int  FAIL_THRESHOLD    = 3;     ///< consecutive missed replies before declaring a daemon hung
  constexpr long COOLDOWN_SEC      = 120;   ///< minimum seconds between restarts of the same daemon
  constexpr int  BROKER_SETTLE_MS  = 300;   ///< let the SUB subscription propagate to our PUB before probing
  constexpr int  STARTUP_GRACE_SEC = 30;    ///< delay before the first probe, so a slow cold-boot is not mistaken for a hang
  constexpr int SNAPSHOT_PERIOD_SEC = 300;  ///< seconds between forced-publish probes
  constexpr int SNAPSHOT_WINDOW_MS = 5000;  ///< time to wait for all requested topics to answer a snapshot round

  const std::string DEFAULT_CFG  = "/home/developer/Software/Config/sequencerd.cfg";
  const std::string LOCALHOST    = "127.0.0.1";
  const std::string BROKER_UNIT  = "messaged";    ///< the ZMQ broker unit (no command port)
  const std::string HEALTH_TOPIC = "_ngps_wd";    ///< private topic for the broker round-trip probe

  /// per-unit request key (set true in the Topic::SNAPSHOT payload) and the
  /// topic to watch for that unit's response. telemd is absent -- confirmed
  /// inactive scaffolding (no pub/sub participation anywhere), not merely
  /// omitted. sequencerd has no single Topic::SEQUENCERD, so it requests on
  /// its own daemon name and is answered on Topic::SEQ_DAEMONSTATE, which
  /// publishes unconditionally.
  struct SnapshotTarget {
      std::string request_key;
      std::string response_topic;
  };
  const std::map<std::string, SnapshotTarget> SNAPSHOT_TARGETS = {
      { "tcsd", { Topic::TCSD, Topic::TCSD } },
      { "slitd", { Topic::SLITD, Topic::SLITD } },
      { "camerad", { Topic::CAMERAD, Topic::CAMERAD } },
      { "acamd", { Topic::ACAMD, Topic::ACAMD } },
      { "calibd", { Topic::CALIBD, Topic::CALIBD } },
      { "flexured", { Topic::FLEXURED, Topic::FLEXURED } },
      { "focusd", { Topic::FOCUSD, Topic::FOCUSD } },
      { "powerd", { Topic::POWERD, Topic::POWERD } },
      { "thermald", { Topic::THERMALD, Topic::THERMALD } },
      { "slicecamd", { Topic::SLICECAMD, Topic::SLICECAMD } },
      { "sequencerd", { Daemon::SEQUENCER, Topic::SEQ_DAEMONSTATE } },
  };
}


/***** trim *****************************************************************/
/**
 * @brief      remove leading and trailing whitespace from a string
 * @param[in]  str  the string to trim
 * @return     the trimmed string (empty if all whitespace)
 *
 */
std::string trim( const std::string &str ) {
  const std::size_t first = str.find_first_not_of( " \t\r\n" );
  if ( first == std::string::npos ) return "";
  const std::size_t last = str.find_last_not_of( " \t\r\n" );
  return str.substr( first, last - first + 1 );
}
/***** trim *****************************************************************/


/***** logmsg *************************************************************/
/**
 * @brief      write a timestamped message to stderr (captured by systemd journal)
 * @param[in]  message  the message to log
 * @return     none
 *
 */
void logmsg( const std::string &message ) {
  const std::time_t now = std::time( nullptr );
  std::tm tmbuf{};
  gmtime_r( &now, &tmbuf );
  char stamp[32];
  std::strftime( stamp, sizeof(stamp), "%Y-%m-%dT%H:%M:%S", &tmbuf );
  std::cerr << stamp << "Z (ngps-watchdog) " << message << std::endl;
}
/***** logmsg *************************************************************/


/***** load_targets *********************************************************/
/**
 * @brief      parse the sequencer config file into maps of unit name to command port
 * @details    Peer daemons appear as "<NAME>D_PORT" (blocking) and
 *             "<NAME>D_NBPORT" (non-blocking); the sequencer's own ports are
 *             "BLKPORT"/"NBPORT". Non-numeric values, such as unresolved CMake
 *             "@TOKEN@" placeholders, are skipped. The broker (messaged) has no
 *             command port and is handled separately (see cfg_value).
 * @param[in]  cfgpath     path to the sequencer config file
 * @param[out] error       set to a message if the file cannot be opened
 * @param[out] nb_targets  map of unit name to non-blocking port, for units that have one
 * @return     map of unit name (e.g. "acamd") to blocking command port
 *
 */
std::map<std::string, uint16_t> load_targets( const std::string &cfgpath, std::string &error,
                                              std::map<std::string, uint16_t> &nb_targets ) {
  std::map<std::string, uint16_t> targets;
  std::ifstream cfg( cfgpath );
  if ( !cfg.is_open() ) { error = "cannot open " + cfgpath; return targets; }

  const std::string suffix = "_PORT";
  const std::string nb_suffix = "_NBPORT";
  std::string line;
  while ( std::getline( cfg, line ) ) {
    const std::size_t hash = line.find( '#' );          // strip an inline comment
    if ( hash != std::string::npos ) line.erase( hash );

    const std::size_t equ = line.find( '=' );
    if ( equ == std::string::npos ) continue;

    const std::string key = trim( line.substr( 0, equ ) );
    const std::string value = trim( line.substr( equ + 1 ) );
    if ( key.empty() || value.empty() ) continue;

    std::string unit;
    bool is_nb = false;
    if ( key == "BLKPORT" ) {
      unit = "sequencerd";                              // the sequencer's own command port
    }
    else if ( key == "NBPORT" ) {
      unit = "sequencerd"; // the sequencer's own non-blocking port
      is_nb = true;
    }
    else if ( key.size() > nb_suffix.size() && key.compare( key.size() - nb_suffix.size(), nb_suffix.size(), nb_suffix ) == 0 &&
              key[key.size() - nb_suffix.size() - 1] == 'D' ) {
      unit = key.substr( 0, key.size() - nb_suffix.size() ); // "ACAMD_NBPORT" -> "ACAMD"
      std::transform( unit.begin(), unit.end(), unit.begin(),
                      []( unsigned char chr ) { return static_cast<char>( std::tolower( chr ) ); } );
      is_nb = true;
    }
    else if ( key.size() > suffix.size() && key.compare( key.size() - suffix.size(), suffix.size(), suffix ) == 0 &&
              key[key.size() - suffix.size() - 1] == 'D' ) {
      unit = key.substr( 0, key.size()-suffix.size() ); // "ACAMD_PORT" -> "ACAMD"
      std::transform( unit.begin(), unit.end(), unit.begin(),
                      []( unsigned char chr ){ return static_cast<char>( std::tolower(chr) ); } );
    }
    else {
      continue;
    }

    try {
      const int port = std::stoi( value );
      if ( port > 0 && port <= 65535 ) {
        if ( is_nb )
          nb_targets[unit] = static_cast<uint16_t>( port );
        else
          targets[unit] = static_cast<uint16_t>( port );
      }
    }
    catch ( const std::exception & ) {
      // non-numeric value (e.g. an unresolved "@TOKEN@"); skip it
    }
  }
  return targets;
}
/***** load_targets *********************************************************/


/***** cfg_value ************************************************************/
/**
 * @brief      return the value of a single key from the config file
 * @details    Used to read the broker endpoints (PUB_ENDPOINT/SUB_ENDPOINT).
 *             Surrounding double quotes are stripped.
 * @param[in]  cfgpath  path to the sequencer config file
 * @param[in]  key      config key to look up
 * @return     the value, or an empty string if not found
 *
 */
std::string cfg_value( const std::string &cfgpath, const std::string &key ) {
  std::ifstream cfg( cfgpath );
  std::string line;
  while ( std::getline( cfg, line ) ) {
    const std::size_t hash = line.find( '#' );
    if ( hash != std::string::npos ) line.erase( hash );
    const std::size_t equ = line.find( '=' );
    if ( equ == std::string::npos ) continue;
    if ( trim( line.substr( 0, equ ) ) != key ) continue;
    std::string value = trim( line.substr( equ + 1 ) );
    if ( value.size() >= 2 && value.front() == '"' && value.back() == '"' ) {
      value = value.substr( 1, value.size() - 2 );      // strip surrounding quotes
    }
    return value;
  }
  return "";
}
/***** cfg_value ************************************************************/


/***** probe_daemon *********************************************************/
/**
 * @brief      probe a daemon's command port for responsiveness
 * @details    Connects, sends "ping", and waits for a "pong" reply within the
 *             timeout. A refused connection means the process is down (handled by
 *             systemd Restart=, not this tool); connect-ok-but-no-reply means hung.
 * @param[in]  port  the daemon's blocking command port
 * @return     true if the daemon replied "pong" within the timeout, else false
 *
 */
bool probe_daemon( uint16_t port ) {
  const int fd = ::socket( AF_INET, SOCK_STREAM, 0 );
  if ( fd < 0 ) return false;

  struct sockaddr_in addr;
  std::memset( &addr, 0, sizeof(addr) );
  addr.sin_family = AF_INET;
  addr.sin_port   = htons( port );
  ::inet_pton( AF_INET, LOCALHOST.c_str(), &addr.sin_addr );

  bool responsive = false;
  if ( ::connect( fd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr) ) == 0 ) {
    const std::string probe = CMD_PING + "\n";
    if ( ::send( fd, probe.c_str(), probe.size(), MSG_NOSIGNAL ) > 0 ) {
      struct pollfd pfd;
      pfd.fd = fd;
      pfd.events = POLLIN;
      pfd.revents = 0;
      if ( ::poll( &pfd, 1, PROBE_TIMEOUT_MS ) > 0 && ( pfd.revents & POLLIN ) ) {
        char buf[64];
        const ssize_t nread = ::recv( fd, buf, sizeof(buf)-1, 0 );
        if ( nread > 0 ) {
          buf[nread] = '\0';
          if ( std::string(buf).find(CMD_PONG) != std::string::npos ) responsive = true;
        }
      }
    }
  }
  ::close( fd );
  return responsive;
}
/***** probe_daemon *********************************************************/


/***** probe_round ***********************************************************/
/**
 * @brief      one broker-health probe, and optionally one publish-liveness round
 * @details    Always round-trips the broker nonce, exactly as probe_broker did.
 *             On a snapshot round (snapshot_targets non-null), also publishes a
 *             Topic::SNAPSHOT request naming every unit in snapshot_targets,
 *             tagged Key::WATCHDOG. Drains any backlog to empty first, so a
 *             message arriving just after the previous round's deadline is never
 *             misattributed to this round.
 * @param[in]  pub               PUB socket connected to the broker's XSUB endpoint
 * @param[in]  sub               SUB socket connected to the broker's XPUB endpoint
 * @param[in]  poller            poller registered on sub for poll_in
 * @param[in]  nonce             unique token for this round's broker probe
 * @param[in]  snapshot_targets  non-null on a snapshot round: units/topics to request
 * @param[out] topic_seen        cleared, and on a snapshot round populated per response_topic
 * @return     true if the broker nonce round-tripped within the window, else false
 *
 */
bool probe_round( zmqpp::socket &pub, zmqpp::socket &sub, zmqpp::poller &poller, const std::string &nonce,
                  const std::map<std::string, SnapshotTarget>* snapshot_targets, std::map<std::string, bool> &topic_seen ) {
  topic_seen.clear();
  bool broker_responsive = false;

  try {
    while ( poller.poll( 0 ) > 0 ) {   // drain any backlog left from the previous round's deadline
      zmqpp::message discard;
      sub.receive( discard );
    }

    zmqpp::message health_out;
    health_out.add( HEALTH_TOPIC );
    health_out.add( nonce );
    pub.send( health_out );

    int window_ms = PROBE_TIMEOUT_MS;

    if ( snapshot_targets ) {          // snapshot round: also ask every listed unit to publish
      nlohmann::json jmessage;
      for ( const auto &[unit, tgt] : *snapshot_targets )
        jmessage[tgt.request_key] = true;
      jmessage[Key::WATCHDOG] = true;  // tags this as a cheap ping, not organic telemetry
      for ( const auto &[unit, tgt] : *snapshot_targets )
        topic_seen[tgt.response_topic] = false;

      zmqpp::message snap_out;
      snap_out.add( Topic::SNAPSHOT );
      snap_out.add( jmessage.dump() );
      pub.send( snap_out );

      window_ms = std::max( PROBE_TIMEOUT_MS, SNAPSHOT_WINDOW_MS );  // snapshot replies may lag a plain ping
    }

    const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds( window_ms );
    while ( std::chrono::steady_clock::now() < deadline ) {
      const bool have_all_topics =
          std::all_of( topic_seen.begin(), topic_seen.end(), []( const auto &kv ) { return kv.second; } );
      if ( broker_responsive && have_all_topics ) break;             // nothing left to wait for this round

      const long remaining =
          std::chrono::duration_cast<std::chrono::milliseconds>( deadline - std::chrono::steady_clock::now() ).count();
      if ( poller.poll( remaining > 0 ? static_cast<int>( remaining ) : 0 ) <= 0 ) break;  // timeout

      zmqpp::message in;
      sub.receive( in );
      std::string topic, payload;
      in >> topic >> payload;
      // our token round-tripped the broker
      if ( topic == HEALTH_TOPIC && payload == nonce ) {
        broker_responsive = true;
        continue;
      }
      auto it = topic_seen.find( topic );
      if ( it != topic_seen.end() ) it->second = true;  // mark this daemon's snapshot reply as seen
    }
  }
  catch ( const std::exception &e ) {
    logmsg( "broker probe error: " + std::string( e.what() ) );
  }
  return broker_responsive;
}
/***** probe_round ***********************************************************/


/***** unit_is_active *******************************************************/
/**
 * @brief      ask systemd whether a unit is currently active
 * @details    Used to skip daemons that are intentionally stopped or mid-restart,
 *             so the watchdog never fights a commanded state change. Unit names
 *             come from the config file (fixed alphabetic keys), so the composed
 *             command is not subject to injection.
 * @param[in]  unit  the daemon unit name (e.g. "acamd")
 * @return     true if "systemctl is-active" reports active, else false
 *
 */
bool unit_is_active( const std::string &unit ) {
  const std::string cmd = "systemctl is-active --quiet ngps@" + unit + ".service";
  const int rc = std::system( cmd.c_str() );
  return ( rc != -1 && WIFEXITED(rc) && WEXITSTATUS(rc) == 0 );
}
/***** unit_is_active *******************************************************/


/***** restart_unit *********************************************************/
/**
 * @brief      restart a daemon unit via systemctl
 * @details    Use "--no-block" to prevent ngps-watchdog.service WatchdogSec
 *             from killing the watchdog itself mid-restart.
 * @param[in]  unit  the daemon unit name (e.g. "acamd")
 * @return     none
 *
 */
void restart_unit( const std::string &unit ) {
  const std::string cmd = "systemctl restart --no-block ngps@" + unit + ".service";
  if ( std::system( cmd.c_str() ) == -1 ) {
    logmsg( "ERROR could not invoke systemctl to restart ngps@" + unit );
  }
}
/***** restart_unit *********************************************************/


/***** consider_restart *****************************************************/
/**
 * @brief      apply the fail-threshold + cooldown policy and restart if warranted
 * @param[in]  unit            the daemon unit name
 * @param[in]  responsive      result of this round's probe
 * @param[in,out] fails        per-unit consecutive-failure counters
 * @param[in,out] last_restart per-unit last-restart timestamps
 * @return     none
 *
 */
void consider_restart( const std::string &unit, bool responsive,
                       std::map<std::string,int> &fails,
                       std::map<std::string,std::chrono::steady_clock::time_point> &last_restart ) {
  if ( responsive ) { fails[unit] = 0; return; }

  ++fails[unit];
  logmsg( "ngps@" + unit + " no reply (" +
          std::to_string(fails[unit]) + "/" + std::to_string(FAIL_THRESHOLD) + ")" );

  const auto now = std::chrono::steady_clock::now();
  const long since = std::chrono::duration_cast<std::chrono::seconds>( now - last_restart[unit] ).count();
  if ( fails[unit] >= FAIL_THRESHOLD && since > COOLDOWN_SEC ) {
    logmsg( "ngps@" + unit + " appears hung -> systemctl restart" );
    restart_unit( unit );
    last_restart[unit] = now;
    fails[unit] = 0;
  }
}
/***** consider_restart *****************************************************/


/***** main *****************************************************************/
/**
 * @brief      watchdog entry point and probe loop
 * @param[in]  argc  argument count
 * @param[in]  argv  argument vector; optional argv[1] overrides the config path
 * @return     1 on fatal startup error, otherwise runs until terminated
 *
 */
int main( int argc, char **argv ) {
  const std::string cfgfile = ( argc > 1 ) ? std::string(argv[1]) : DEFAULT_CFG;

  std::string error;
  std::map<std::string, uint16_t> nb_targets;
  const std::map<std::string, uint16_t> targets = load_targets( cfgfile, error, nb_targets );
  if ( !error.empty() ) { logmsg( "ERROR " + error ); return 1; }
  if ( targets.empty() ) { logmsg( "ERROR no daemon command ports found in " + cfgfile ); return 1; }

  std::string watched;
  for ( const auto &target : targets ) watched += ( watched.empty() ? "" : ", " ) + target.first;
  logmsg( "watching command ports: " + watched );

  std::string watched_nb;
  for ( const auto &target : nb_targets )
    watched_nb += ( watched_nb.empty() ? "" : ", " ) + target.first;
  logmsg( "watching non-blocking ports: " + watched_nb );

  std::map<std::string, int> fails;
  std::map<std::string, std::chrono::steady_clock::time_point> last_restart;
  const auto startup = std::chrono::steady_clock::now();
  for ( const auto &target : targets ) {
    fails[target.first] = 0;
    last_restart[target.first] = startup - std::chrono::seconds( COOLDOWN_SEC + 1 );
  }

  std::map<std::string, int> pub_fails;
  for ( const auto &target : SNAPSHOT_TARGETS ) {
    pub_fails[target.first] = 0;
    last_restart[target.first] = startup - std::chrono::seconds( COOLDOWN_SEC + 1 );
  }

  std::map<std::string, int> nb_fails;
  for ( const auto &target : nb_targets ) {
    nb_fails[target.first] = 0;
    last_restart[target.first] = startup - std::chrono::seconds( COOLDOWN_SEC + 1 );
  }

  // Set up the broker (messaged) round-trip probe, if its endpoints are
  // configured. PUB_ENDPOINT is where publishers connect (broker XSUB) and
  // SUB_ENDPOINT is where subscribers connect (broker XPUB). The sockets are
  // persistent and we settle once so the subscription propagates to our PUB.
  //
  const std::string broker_pub_ep = cfg_value( cfgfile, "PUB_ENDPOINT" );
  const std::string broker_sub_ep = cfg_value( cfgfile, "SUB_ENDPOINT" );
  bool broker_enabled = ( !broker_pub_ep.empty() && !broker_sub_ep.empty() );

  zmqpp::context zmq_context;
  zmqpp::socket broker_pub( zmq_context, zmqpp::socket_type::publish );
  zmqpp::socket broker_sub( zmq_context, zmqpp::socket_type::subscribe );
  zmqpp::poller broker_poller;
  unsigned long nonce = 0;

  if ( broker_enabled ) {
    try {
      broker_pub.connect( broker_pub_ep );
      broker_sub.connect( broker_sub_ep );
      broker_sub.subscribe( HEALTH_TOPIC );
      // also listen for each daemon's Topic::SNAPSHOT reply
      for ( const auto &target : SNAPSHOT_TARGETS )
        broker_sub.subscribe( target.second.response_topic );
      broker_poller.add( broker_sub, zmqpp::poller::poll_in );
      std::this_thread::sleep_for( std::chrono::milliseconds( BROKER_SETTLE_MS ) );
      fails[BROKER_UNIT] = 0;
      last_restart[BROKER_UNIT] = startup - std::chrono::seconds( COOLDOWN_SEC + 1 );
      logmsg( "watching broker " + BROKER_UNIT + " round-trip " + broker_pub_ep + " -> " + broker_sub_ep );

      std::string watched_pub;
      for ( const auto &target : SNAPSHOT_TARGETS )
        watched_pub += ( watched_pub.empty() ? "" : ", " ) + target.first;
      logmsg( "watching publish liveness (Topic::SNAPSHOT, every " + std::to_string( SNAPSHOT_PERIOD_SEC ) +
              "s) for: " + watched_pub );
    }
    catch ( const std::exception &e ) {
      logmsg( "ERROR setting up broker probe (" + std::string(e.what()) + "); " + BROKER_UNIT + " not hang-probed" );
      broker_enabled = false;
    }
  }
  else {
    logmsg( "broker endpoints not in config; " + BROKER_UNIT + " and publish-liveness will not be hang-probed" );
  }

  // Startup grace: give daemons time to come up before probing for liveness, so
  // a slow first start (sequencerd opening many connections, cameras enumerating
  // hardware) is not mistaken for a hang. systemd's WatchdogSec= is heartbeated
  // throughout the wait so the watchdog itself remains supervised.
  //
  logmsg( "startup grace " + std::to_string( STARTUP_GRACE_SEC ) + "s before first probe" );
  {
    const auto grace_end = std::chrono::steady_clock::now() + std::chrono::seconds( STARTUP_GRACE_SEC );
    while ( std::chrono::steady_clock::now() < grace_end ) {
      Daemon::sd_notify( "WATCHDOG=1\n" );
      std::this_thread::sleep_for( std::chrono::seconds( PROBE_PERIOD_SEC ) );
    }
  }

  auto next_snapshot = std::chrono::steady_clock::now();
  std::map<std::string, bool> topic_seen;

  while ( true ) {
    for ( const auto &target : targets ) {
      const std::string &unit = target.first;
      const bool active = unit_is_active( unit );
      const bool responsive = active ? probe_daemon( target.second ) : true;
      consider_restart( unit, responsive, fails, last_restart );
      Daemon::sd_notify( "WATCHDOG=1\n" );   // heartbeat tied to real probe progress

      const auto nb_it = nb_targets.find( unit );
      if ( nb_it != nb_targets.end() ) {
        const bool nb_responsive = active ? probe_daemon( nb_it->second ) : true;
        consider_restart( unit, nb_responsive, nb_fails, last_restart );
        Daemon::sd_notify( "WATCHDOG=1\n" );
      }
    }

    if ( broker_enabled ) {
      const bool broker_active = unit_is_active( BROKER_UNIT );
      const auto now = std::chrono::steady_clock::now();
      const bool snapshot_due = broker_active && ( now >= next_snapshot );  // deferred, not penalized, if messaged is down

      const bool broker_responsive = broker_active
                                         ? probe_round( broker_pub, broker_sub, broker_poller, std::to_string( ++nonce ),
                                                        snapshot_due ? &SNAPSHOT_TARGETS : nullptr, topic_seen )
                                         : true;
      consider_restart( BROKER_UNIT, broker_responsive, fails, last_restart );
      Daemon::sd_notify( "WATCHDOG=1\n" );

      if ( snapshot_due ) {
        for ( const auto &[unit, tgt] : SNAPSHOT_TARGETS ) {
          const bool responsive = unit_is_active( unit ) ? topic_seen[tgt.response_topic] : true;
          consider_restart( unit, responsive, pub_fails, last_restart );
        }
        next_snapshot = now + std::chrono::seconds( SNAPSHOT_PERIOD_SEC );  // only advance if the probe actually ran
      }
    }

    std::this_thread::sleep_for( std::chrono::seconds( PROBE_PERIOD_SEC ) );
  }

  return 0;   // not reached
}
/***** main *****************************************************************/
