/**
 * @file    calib.h
 * @brief   
 * @details 
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */
#ifndef CALIB_H
#define CALIB_H

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

/***** CalibEmulator **********************************************************/
/**
 * @namespace CalibEmulator
 * @brief     this namespace contains everything for the calibrator emulator
 *
 */
namespace CalibEmulator {

  /***** CalibEmulator::ControllerInfo ****************************************/
  /**
   * @class  ControllerInfo
   * @brief  
   *
   */
  class ControllerInfo {
    public:
      bool homed;
      bool ontarget;
      float pos;

      int addr;
      std::string name;
      float min, max;

      ControllerInfo() : homed(false), ontarget(false), pos(-1) { }

      long load_info( std::string &input ) {
        std::string function = "  (CalibEmulator::ControllerInfo::load_info) ";
        std::vector<std::string> tokens;

        Tokenize( input, tokens, " \"" );

        if ( tokens.size() < 1 || tokens.size() > 5 ) {
          std::cerr << get_timestamp() << function << "bad number of tokens: " << tokens.size() << ". expected 5\n";
          return( ERROR );
        }

        try {
          this->addr = std::stoi( tokens.at(3) );
          this->name = tokens.at(0);
        }
        catch ( std::invalid_argument &e ) {
          std::cerr << get_timestamp() << function << "error loading tokens from input: " << input << ": " << e.what() << "\n";
        }
        catch ( std::out_of_range &e ) {
          std::cerr << get_timestamp() << function << "error loading tokens from input: " << input << ": " << e.what() << "\n";
        }

        return( NO_ERROR );
      }

  };
  /***** CalibEmulator::ControllerInfo ****************************************/


  /***** CalibEmulator::Interface *********************************************/
  /**
   * @class  Interface
   * @brief  
   *
   */
  class Interface {
    public:
      Interface() = default;

      std::mutex pos_mutex;

      // This is a vector of all daisy-chain connected controllers
      //
      std::vector<CalibEmulator::ControllerInfo> controller_info;

      long parse_command( std::string cmd, std::string &retstring );
      long test();
      static void do_home( CalibEmulator::ControllerInfo &info, std::mutex &mlock );
      static void do_move( CalibEmulator::ControllerInfo &info, std::mutex &mlock, int distance, float pos );

  };
  /***** CalibEmulator::Interface *********************************************/

}
/***** CalibEmulator **********************************************************/
#endif
