/**
 * @file    acam.h
 * @brief   
 * @details 
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */
#ifndef ACAM_H
#define ACAM_H

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

/***** AcamEmulator ***********************************************************/
/**
 * @namespace AcamEmulator
 * @brief     this namespace contains everything for the acam emulator
 *
 */
namespace AcamEmulator {

  /***** AcamEmulator::ControllerInfo *****************************************/
  /**
   * @class  ControllerInfo
   * @brief  emulated motor controller info class
   *
   * This class contains the info for the emulated motor controllers
   *
   */
  class ControllerInfo {
    private:

    public:
      ControllerInfo();

      bool servo;
      bool homed;
      bool ontarget;
      int addr;
      std::string name;
      float min, max;
      float pos;

      long load_info( std::string &input ) {
        std::string function = "  (AcamEmulator::ControllerInfo::load_info) ";
        std::vector<std::string> tokens;

        Tokenize( input, tokens, " \"" );

        if ( tokens.size() < 1 || tokens.size() > 5 ) {
          std::cerr << get_timestamp() << function << "bad number of tokens: " << tokens.size() << ". expected 5\n";
          return( ERROR );
        }

        try {
          this->addr = std::stoi( tokens.at(3) );
          this->name = tokens.at(0);
//        this->min =  std::stof( tokens.at(2) );
//        this->max =  std::stof( tokens.at(3) );
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
  /***** AcamEmulator::ControllerInfo *****************************************/


  /***** AcamEmulator::Interface **********************************************/
  /**
   * @class  Interface
   * @brief  interface class for the acam emulator
   *
   * This class interfaces the daemon to the emulated controller.
   *
   */
  class Interface {
    public:
      Interface() = default;

      std::mutex pos_mutex;

      // This is a vector of all daisy-chain connected controllers
      //
      std::vector<AcamEmulator::ControllerInfo> controller_info;

      long parse_command( std::string cmd, std::string &retstring );
      long test();
      static void do_home( AcamEmulator::ControllerInfo &info, std::mutex &mlock );
      static void do_move( AcamEmulator::ControllerInfo &info, std::mutex &mlock, int distance, float pos );

  };
  /***** AcamEmulator::Interface **********************************************/

}
/***** AcamEmulator ***********************************************************/
#endif
