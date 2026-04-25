/**
 * @file    seqmon.cpp
 * @brief   terminal-based subscriber that mimics the sequencer operator display
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 * Subscribes to three sequencer topics on the ZMQ broker and renders a simple
 * color-coded terminal display:
 *
 *   seq_seqstate  lifecycle state (top line, color by value)
 *   seq_waitstate active wait bits (second line, list of set labels)
 *   broadcast     operator messages (scrolling log, color by severity)
 *
 * This is a standalone utility intended as a stand-in for the GUI message
 * window during testing. It has no dependencies on the sequencer sources;
 * it only shares the message_keys.h contract.
 *
 */

#include <zmqpp/zmqpp.hpp>
#include <nlohmann/json.hpp>
#include "message_keys.h"

#include <atomic>
#include <csignal>
#include <ctime>
#include <iostream>
#include <sstream>
#include <string>

namespace {

  constexpr const char *BROKER_ENDPOINT = "tcp://localhost:5556";

  // ANSI escape codes
  //
  constexpr const char *ANSI_RESET    = "\033[0m";
  constexpr const char *ANSI_GRAY     = "\033[90m";
  constexpr const char *ANSI_RED      = "\033[31m";
  constexpr const char *ANSI_GREEN    = "\033[32m";
  constexpr const char *ANSI_YELLOW   = "\033[33m";
  constexpr const char *ANSI_BLUE     = "\033[34m";
  constexpr const char *ANSI_CYAN     = "\033[36m";
  constexpr const char *ANSI_BOLDYEL  = "\033[1;33m";

  // cursor control: save/restore, move up 3 lines, clear entire line
  //
  constexpr const char *CURSOR_SAVE    = "\033[s";
  constexpr const char *CURSOR_RESTORE = "\033[u";
  constexpr const char *CURSOR_HOME    = "\033[H";
  constexpr const char *CLEAR_SCREEN   = "\033[2J";
  constexpr const char *CLEAR_LINE     = "\033[2K";

  std::atomic<bool> running{true};

  // Cached last-known state so a redraw after a broadcast message can
  // reprint the fixed top lines.
  //
  std::string last_seqstate = "(unknown)";
  std::string last_waitstate;

  /***** color_for_state ******************************************************/
  /**
   * @brief   map a lifecycle state label to an ANSI color
   * @param   state  state string (e.g. "NOTREADY", "RUNNING")
   * @return  ANSI escape sequence
   */
  const char *color_for_state( const std::string &state ) {
    if ( state.find("FAILED")   != std::string::npos ) return ANSI_RED;
    if ( state.find("ABORTING") != std::string::npos ) return ANSI_BOLDYEL;
    if ( state.find("RUNNING")  != std::string::npos ) return ANSI_GREEN;
    if ( state.find("READY")    != std::string::npos ) return ANSI_YELLOW;
    if ( state.find("STARTING") != std::string::npos ) return ANSI_BLUE;
    if ( state.find("STOPPING") != std::string::npos ) return ANSI_BLUE;
    if ( state.find("PAUSED")   != std::string::npos ) return ANSI_CYAN;
    if ( state.find("NOTREADY") != std::string::npos ) return ANSI_GRAY;
    return ANSI_RESET;
  }
  /***** color_for_state ******************************************************/


  /***** color_for_severity ***************************************************/
  /**
   * @brief   map a broadcast severity label to an ANSI color
   * @param   severity  severity string (NOTICE|WARNING|ERROR)
   * @return  ANSI escape sequence
   */
  const char *color_for_severity( const std::string &severity ) {
    if ( severity == Severity::ERROR )   return ANSI_RED;
    if ( severity == Severity::WARNING ) return ANSI_YELLOW;
    return ANSI_RESET;
  }
  /***** color_for_severity ***************************************************/


  /***** timestamp ************************************************************/
  /**
   * @brief   local time as HH:MM:SS
   * @return  formatted string
   */
  std::string timestamp() {
    std::time_t now = std::time(nullptr);
    std::tm tm_local{};
    localtime_r( &now, &tm_local );
    char buf[16];
    std::strftime( buf, sizeof(buf), "%H:%M:%S", &tm_local );
    return std::string(buf);
  }
  /***** timestamp ************************************************************/


  /***** redraw_status ********************************************************/
  /**
   * @brief   reprint the fixed status header (lines 1 and 2) in place
   * @details Uses ANSI save/restore so the current scroll position for
   *         broadcast messages is not disturbed.
   */
  void redraw_status() {
    std::cout << CURSOR_SAVE
              << CURSOR_HOME
              << CLEAR_LINE
              << "STATE:   " << color_for_state(last_seqstate)
              << last_seqstate << ANSI_RESET << "\n"
              << CLEAR_LINE
              << "WAITING: " << last_waitstate << "\n"
              << CURSOR_RESTORE
              << std::flush;
  }
  /***** redraw_status ********************************************************/


  /***** handle_seqstate ******************************************************/
  /**
   * @brief   parse a seq_seqstate payload and update the top status line
   * @param   payload  JSON payload as string
   */
  void handle_seqstate( const std::string &payload ) {
    try {
      auto j = nlohmann::json::parse( payload );
      if ( j.contains(Key::Sequencer::SEQSTATE) ) {
        last_seqstate = j[Key::Sequencer::SEQSTATE].get<std::string>();
        if ( last_seqstate.empty() ) last_seqstate = "(none)";
        redraw_status();
      }
    }
    catch ( const std::exception &e ) {
      std::cerr << "seqstate parse error: " << e.what() << "\n";
    }
  }
  /***** handle_seqstate ******************************************************/


  /***** handle_waitstate *****************************************************/
  /**
   * @brief   parse a seq_waitstate payload and update the wait-bit line
   * @param   payload  JSON payload as string
   * @details Payload contains one boolean per wait-bit label; collect the
   *          labels whose value is true.
   */
  void handle_waitstate( const std::string &payload ) {
    try {
      auto j = nlohmann::json::parse( payload );
      std::ostringstream oss;
      bool first = true;
      for ( auto it = j.begin(); it != j.end(); ++it ) {
        if ( ! it.value().is_boolean() ) continue;
        if ( it.value().get<bool>() ) {
          if ( !first ) oss << " ";
          oss << it.key();
          first = false;
        }
      }
      last_waitstate = oss.str();
      if ( last_waitstate.empty() ) last_waitstate = "(idle)";
      redraw_status();
    }
    catch ( const std::exception &e ) {
      std::cerr << "waitstate parse error: " << e.what() << "\n";
    }
  }
  /***** handle_waitstate *****************************************************/


  /***** handle_broadcast *****************************************************/
  /**
   * @brief   parse a broadcast payload and append a colored line to the log
   * @param   payload  JSON payload as string
   */
  void handle_broadcast( const std::string &payload ) {
    try {
      auto j = nlohmann::json::parse( payload );
      std::string severity = j.value( Key::Broadcast::SEVERITY, std::string("NOTICE") );
      std::string message  = j.value( Key::Broadcast::MESSAGE,  std::string("") );
      std::string source   = j.value( Key::SOURCE,              std::string("?") );
      std::cout << color_for_severity(severity)
                << "[" << timestamp() << "] "
                << "[" << source << "] "
                << severity << ": " << message
                << ANSI_RESET << "\n"
                << std::flush;
    }
    catch ( const std::exception &e ) {
      std::cerr << "broadcast parse error: " << e.what() << "\n";
    }
  }
  /***** handle_broadcast *****************************************************/


  /***** signal_handler *******************************************************/
  /**
   * @brief   SIGINT handler; triggers clean exit of the main loop
   */
  void signal_handler( int /*signum*/ ) {
    running.store(false);
  }
  /***** signal_handler *******************************************************/

}  // anonymous namespace


void usage( const char *exe ) {
  std::cout << "usage: " << exe << " [endpoint]\n"
            << "   endpoint  ZMQ broker address (default " << BROKER_ENDPOINT << ")\n";
}


int main( int argc, char *argv[] ) {

  std::string endpoint = BROKER_ENDPOINT;

  if ( argc > 1 ) {
    std::string arg = argv[1];
    if ( arg == "-h" || arg == "--help" ) {
      usage(argv[0]);
      return 0;
    }
    endpoint = arg;
  }

  std::signal( SIGINT,  signal_handler );
  std::signal( SIGTERM, signal_handler );

  // set up ZMQ subscriber
  //
  zmqpp::context context;
  zmqpp::socket  subscriber( context, zmqpp::socket_type::subscribe );

  try {
    subscriber.connect( endpoint );
    subscriber.subscribe( Topic::BROADCAST );
    subscriber.subscribe( Topic::SEQ_SEQSTATE );
    subscriber.subscribe( Topic::SEQ_WAITSTATE );
  }
  catch ( const std::exception &e ) {
    std::cerr << "ERROR connecting to " << endpoint << ": " << e.what() << "\n";
    return 1;
  }

  // initial screen: clear, draw status header, leave room for scrolling log
  //
  std::cout << CLEAR_SCREEN << CURSOR_HOME
            << "STATE:   " << last_seqstate << "\n"
            << "WAITING: " << last_waitstate << "\n"
            << std::string(60, '-') << "\n"
            << "seqmon subscribed to " << endpoint
            << " (Ctrl-C to exit)\n"
            << std::flush;

  // poll so Ctrl-C can break out promptly
  //
  zmqpp::poller poller;
  poller.add( subscriber, zmqpp::poller::poll_in );

  while ( running.load() ) {
    if ( poller.poll(500) == 0 ) continue;
    if ( ! poller.has_input(subscriber) ) continue;

    zmqpp::message zmsg;
    subscriber.receive( zmsg );
    std::string topic, payload;
    zmsg >> topic >> payload;

    if      ( topic == Topic::SEQ_SEQSTATE  ) handle_seqstate( payload );
    else if ( topic == Topic::SEQ_WAITSTATE ) handle_waitstate( payload );
    else if ( topic == Topic::BROADCAST     ) handle_broadcast( payload );
    // unrecognized topics silently ignored
  }

  std::cout << "\nseqmon exiting\n" << std::flush;
  return 0;
}
