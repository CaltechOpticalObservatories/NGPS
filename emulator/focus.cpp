#include "focus.h"
#include <map>
#include <sstream>
#include <iomanip>
#include <cctype>

namespace Focus {
  namespace {
    struct AxisState {
      double pos = 0.0;
      bool homed = false;
      bool ontarget = true;
      bool servo = true;
    };

    std::map<std::pair<int,int>, AxisState> axis_state;

    AxisState &get_state( int addr, int axis ) {
      return axis_state[std::make_pair(addr, axis)];
    }

    bool is_integer_token( const std::string &s ) {
      if ( s.empty() ) return false;
      size_t i = 0;
      if ( s[0] == '-' || s[0] == '+' ) i = 1;
      if ( i >= s.size() ) return false;
      for ( ; i < s.size(); ++i ) {
        if ( !std::isdigit( static_cast<unsigned char>(s[i]) ) ) return false;
      }
      return true;
    }
  }

  /**************** Focus::Interface::Interface *******************************/
  /**
   * @fn         Interface
   * @brief      class constructor
   * @param[in]  none
   * @return     none
   *
   */
  Interface::Interface() {
  }
  /**************** Focus::Interface::Interface *******************************/


  /**************** Focus::Interface::~Interface ******************************/
  /**
   * @fn         ~Interface
   * @brief      class deconstructor
   * @param[in]  none
   * @return     none
   *
   */
  Interface::~Interface() {
  }
  /**************** Focus::Interface::~Interface ******************************/

  /**************** Focus::Interface::parse_command ***************************/
  /**
   * @fn         parse_command
   * @brief      parse commands for the NPS, spawn a thread if necessary
   * @param[in]  cmd
   * @param[out] retstring
   * @return     ERROR or NO_ERROR
   *
   * This function parses the commands and arguments received by the emulator which
   * would need to go to the NPS.
   *
   */
  long Interface::parse_command( std::string cmd, std::string &retstring ) {
    std::string function = "  (Focus::Interface::parse_command) ";

    std::cerr << get_timestamp() << function << "received command: " << cmd << "\n";

    std::vector<std::string> tokens;
    Tokenize( cmd, tokens, " " );

    std::string mycmd;                 // command is the 1st token
    size_t nargs;                      // number of args (after command)

    if ( tokens.size() < 1 ) {         // should be impossible since already checked for cmd.empty()
      std::cerr << get_timestamp() << function << "ERROR: no tokens\n";
      retstring = "-2";                // invalid parameters
      return( ERROR );
    }

    try {
      // optional leading address
      int addr = 1;
      size_t idx = 0;
      if ( is_integer_token( tokens.at(0) ) ) {
        addr = std::stoi( tokens.at(0) );
        idx++;
      }
      if ( idx >= tokens.size() ) {
        retstring.clear();
        return NO_ERROR;
      }

      mycmd = tokens.at(idx);          // command token
      idx++;
      nargs = tokens.size() - idx;     // number of args after removing command

      // normalize command
      std::transform( mycmd.begin(), mycmd.end(), mycmd.begin(), ::toupper );

      auto get_axis = [&](int fallback=1) -> int {
        if ( idx < tokens.size() && is_integer_token( tokens.at(idx) ) ) {
          int axis = std::stoi( tokens.at(idx) );
          idx++;
          return axis;
        }
        return fallback;
      };

      // ERR? -> "0 <addr> 0"
      if ( mycmd == "ERR?" ) {
        retstring = "0 " + std::to_string(addr) + " 0";
      }
      else
      if ( mycmd == "SVO?" ) {
        int axis = get_axis();
        auto &state = get_state( addr, axis );
        retstring = "0 " + std::to_string(addr) + " " + std::to_string(axis) + "=" + ( state.servo ? "1" : "0" );
      }
      else
      if ( mycmd == "SVO" ) {
        int axis = 1;
        if ( idx + 1 <= tokens.size() && idx < tokens.size() && is_integer_token( tokens.at(idx) ) ) {
          axis = std::stoi( tokens.at(idx) );
          idx++;
        }
        if ( idx < tokens.size() ) {
          int val = 1;
          try { val = std::stoi( tokens.at(idx) ); } catch (...) { val = 1; }
          get_state( addr, axis ).servo = ( val != 0 );
        }
        retstring.clear();
      }
      else
      if ( mycmd == "RON" ) {
        // referencing on; no reply needed
        retstring.clear();
      }
      else
      if ( mycmd == "FRF" || mycmd == "FNL" || mycmd == "FPL" ) {
        int axis = get_axis();
        auto &state = get_state( addr, axis );
        state.homed = true;
        state.ontarget = true;
        retstring.clear();
      }
      else
      if ( mycmd == "FRF?" ) {
        int axis = get_axis();
        auto &state = get_state( addr, axis );
        retstring = "0 " + std::to_string(addr) + " " + std::to_string(axis) + "=" + ( state.homed ? "1" : "0" );
      }
      else
      if ( mycmd == "ONT?" ) {
        int axis = get_axis();
        auto &state = get_state( addr, axis );
        retstring = "0 " + std::to_string(addr) + " " + std::to_string(axis) + "=" + ( state.ontarget ? "1" : "0" );
      }
      else
      if ( mycmd == "POS?" ) {
        int axis = get_axis();
        auto &state = get_state( addr, axis );
        std::ostringstream os;
        os << "0 " << addr << " " << axis << "=" << std::fixed << std::setprecision(3) << state.pos;
        retstring = os.str();
      }
      else
      if ( mycmd == "MOV" || mycmd == "MVR" ) {
        int axis = 1;
        double pos = 0.0;
        if ( idx < tokens.size() ) {
          if ( (tokens.size() - idx) >= 2 && is_integer_token( tokens.at(idx) ) ) {
            axis = std::stoi( tokens.at(idx) );
            idx++;
          }
          if ( idx < tokens.size() ) {
            try { pos = std::stod( tokens.at(idx) ); } catch (...) { pos = 0.0; }
          }
        }
        auto &state = get_state( addr, axis );
        if ( mycmd == "MOV" ) state.pos = pos;
        else state.pos += pos;
        state.ontarget = true;
        retstring.clear();
      }
      else
      if ( mycmd == "STP" ) {
        int axis = get_axis();
        get_state( addr, axis ).ontarget = true;
        retstring.clear();
      }
      else {
        // unknown command; respond minimally if this was a query
        if ( mycmd.find('?') != std::string::npos ) {
          int axis = get_axis();
          retstring = "0 " + std::to_string(addr) + " " + std::to_string(axis) + "=0";
        }
        else {
          retstring.clear();
        }
      }
    }
    catch( std::invalid_argument &e ) {
      std::cerr << get_timestamp() << function << "unable to convert one or more values: " << e.what() << "\n";
      retstring = "-1";                // unrecognized command
      return( ERROR );
    }
    catch( std::out_of_range &e ) {
      std::cerr << get_timestamp() << function << "one or more values out of range: " << e.what() << "\n";
      retstring = "-1";                // unrecognized command
      return( ERROR );
    }

    /* ------------------------
     * handle the commands here
     * ------------------------
     *
     */

    std::cerr << get_timestamp() << function << "reply from focus emulator: " << retstring << "\n";

    return ( NO_ERROR );
  }
  /**************** Focus::Interface::parse_command ***************************/

}
