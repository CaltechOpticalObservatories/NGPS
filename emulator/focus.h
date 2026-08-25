/**
 * @file    focus.h
 * @brief   
 * @details 
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */
#ifndef FOCUS_H
#define FOCUS_H

#include <atomic>
#include <mutex>
#include <chrono>
#include <numeric>
#include <functional>
#include <thread>
#include <fenv.h>
#include <iostream>
#include <vector>
#include <algorithm>

#include "utilities.h"
#include "common.h"
#include "config.h"
#include "logentry.h"
#include "network.h"

namespace Focus {

  /***** Focus::ControllerInfo *************************************************/
  /**
   * @class  ControllerInfo
   * @brief  holds the emulated state for one PI-type focus controller
   *
   */
  class ControllerInfo {
    public:
      bool homed;
      bool ontarget;
      float pos;

      int addr;
      std::string name;

      ControllerInfo() : homed( false ), ontarget( false ), pos( -1 ), addr( -1 ) {}

      // MOTOR_CONTROLLER format for focusd is
      // "<name> <type> <host> <port> <addr> <axes>" -- note the extra <type>
      // field (PI|GALIL) that calibd/slitd don't have. Only PI entries are
      // loaded here; GALIL (U) has no protocol emulation yet.
      //
      long load_info( std::string &input ) {
        std::string function = "  (Focus::ControllerInfo::load_info) ";
        std::vector<std::string> tokens;

        Tokenize( input, tokens, " \"" );

        if ( tokens.size() != 6 ) {
          std::cerr << get_timestamp() << function << "bad number of tokens: " << tokens.size() << ". expected 6\n";
          return ( ERROR );
        }

        if ( tokens.at( 1 ) != "PI" ) {
          return ( NO_ERROR ); // not an error -- just nothing to load (e.g. GALIL)
        }

        try {
          this->name = tokens.at( 0 );
          this->addr = std::stoi( tokens.at( 4 ) );
        }
        catch ( std::invalid_argument &e ) {
          std::cerr << get_timestamp() << function << "error loading tokens from input: " << input << ": " << e.what() << "\n";
          return ( ERROR );
        }
        catch ( std::out_of_range &e ) {
          std::cerr << get_timestamp() << function << "error loading tokens from input: " << input << ": " << e.what() << "\n";
          return ( ERROR );
        }

        return ( NO_ERROR );
      }
  };
  /***** Focus::ControllerInfo *************************************************/


  /** Interface ***************************************************************/
  /**
   * @class  Interface
   * @brief
   *
   */
  class Interface {
    private:
    public:
      Interface();
      ~Interface();

      std::mutex pos_mutex;

      // This is a vector of the PI-type controllers only (not U/GALIL)
      //
      std::vector<Focus::ControllerInfo> controller_info;

      long parse_command( std::string cmd, std::string &retstring );
      static void do_home( Focus::ControllerInfo &info, std::mutex &mlock );
      static void do_move( Focus::ControllerInfo &info, std::mutex &mlock, int distance, float pos );
  };
  /** Interface ***************************************************************/

}
#endif
