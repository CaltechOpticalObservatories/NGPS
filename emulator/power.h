/**
 * @file    power.h
 * @brief   
 * @details 
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */
#ifndef POWER_H
#define POWER_H

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
#include <map>

#include "utilities.h"
#include "common.h"
#include "config.h"
#include "logentry.h"
#include "network.h"

namespace Power {

  /** NpsInfo *****************************************************************/
  /**
   * @class  NpsInfo
   * @brief  NPS information class
   *
   * This class defines the information for each NPS
   *
   */
  class NpsInfo {
    private:
    public:
      int         npsnum;            /// an integer assigned to each NPS as an identifier
      int         maxplugs;          /// the number of outlets on this NPS
      std::string host;              /// host name/IP for this NPS
      int         port;              /// port number for this NPS

//    std::vector<int> plugstate;    /// vector of plug states supported by this NPS
      std::map<int, int> plugstate;  /// STL map of plug states for this NPS indexed by plugnum

      NpsInfo();
      ~NpsInfo();

      /**************** Power::NpsInfo::load_nps_info *************************/
      /**
       * @fn         load_nps_info
       * @brief      loads NPS information from the configuration file into the class
       * @param[in]  input
       * @return     ERROR or NO_ERROR
       *
       * This function is called whenever the NPS_UNIT key is found in the
       * configuration file, to parse and load all of the information assigned
       * by that key into the appropriate NpsInfo class variables.
       *
       * The input string specifies: "<nps#> <maxplugs> <host> <port>"
       *
       */
      long load_nps_info( std::string &input, int &npsnum ) {
        std::string function = "Power::NpsInfo::load_nps_info";
        std::stringstream message;
        std::vector<std::string> tokens;

        Tokenize( input, tokens, " \"" );

        if ( tokens.size() != 4 ) {
          message.str(""); message << "ERROR bad number of tokens: " << tokens.size() << ". expected 4";
          logwrite( function, message.str() );
          return( ERROR );
        }

        try {
          npsnum         = std::stoi( tokens.at(0) );
          this->maxplugs = std::stoi( tokens.at(1) );
          this->host     = tokens.at(2);
          this->port     = std::stoi( tokens.at(3) );
        }
        catch ( std::invalid_argument &e ) {
          message.str(""); message << "ERROR loading tokens from input: " << input << ": " << e.what();
          logwrite( function, message.str() );
          return( ERROR );
        }
        catch ( std::out_of_range &e ) {
          message.str(""); message << "ERROR loading tokens from input: " << input << ": " << e.what();
          logwrite( function, message.str() );
        }

        if ( npsnum < 0 ) {
          message.str(""); message << "ERROR bad NPS unit number " << npsnum << ": must be >= 0";
          logwrite( function, message.str() );
          return( ERROR );
        }
        else this->npsnum = npsnum;

        return( NO_ERROR );
      }
      /**************** Power::NpsInfo::load_nps_info *************************/


      /**************** Power::NpsInfo::load_plug_info ************************/
      /**
       * @fn         load_plug_info
       * @brief      loads plug information from the configuration file into the class
       * @param[in]  input
       * @return     ERROR or NO_ERROR
       *
       * This function is called whenever the NPS_PLUG key is found in the
       * configuration file, to parse and load all of the information assigned
       * by that key into the appropriate NpsInfo class variables.
       *
       * The input string specifies: "<nps#> <plug#> <plugname>"
       *
       */
      long load_plug_info( std::string &input, int &npsnum, int &plugnum, std::string &plugname ) {
        std::string function = "Power::NpsInfo::load_plug_info";
        std::stringstream message;
        std::vector<std::string> tokens;

        Tokenize( input, tokens, " \"" );

        if ( tokens.size() != 3 ) {
          message.str(""); message << "ERROR bad number of tokens in \"" << input << "\": expected 3 but received " << tokens.size();
          logwrite( function, message.str() );
          return( ERROR );
        }

        try {
          npsnum      = std::stoi( tokens.at(0) );
          plugnum     = std::stoi( tokens.at(1) );
          plugname    = tokens.at(2);
        }
        catch ( std::invalid_argument &e ) {
          message.str(""); message << "ERROR loading tokens from input: " << input << ": " << e.what();
          logwrite( function, message.str() );
          return( ERROR );
        }
        catch ( std::out_of_range &e ) {
          message.str(""); message << "ERROR loading tokens from input: " << input << ": " << e.what();
          logwrite( function, message.str() );
        }

        if ( npsnum < 0 ) {
          message.str(""); message << "ERROR bad NPS unit number " << npsnum << ": must be >= 0";
          logwrite( function, message.str() );
          return( ERROR );
        }

        if ( plugnum < 0 ) {
          message.str(""); message << "ERROR bad plug number " << plugnum << ": must be >= 0";
          logwrite( function, message.str() );
          return( ERROR );
        }

        if ( plugname.empty() ) {
          logwrite( function, "ERROR plug name cannot be empty" );
          return( ERROR );
        }

        return( NO_ERROR );
      }
      /**************** Power::NpsInfo::load_plug_info ************************/

  };
  /** NpsInfo *****************************************************************/


  /** Interface ***************************************************************/
  /**
   * @class  Interface
   * @brief  interface to the emulated "hardware"
   *
   */
  class Interface {
    private:
    public:
      Interface();
      ~Interface();

      std::map< int, Power::NpsInfo > nps_info;            /// STL map of NpsInfo objects indexed by NPS unit#

      long parse_command( int npsnum, std::string cmd, std::string &retstring );

  };
  /** Interface ***************************************************************/

}
#endif
