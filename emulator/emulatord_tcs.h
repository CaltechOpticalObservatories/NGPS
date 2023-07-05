/** ---------------------------------------------------------------------------
 * @file     emulatord_tcs.h
 * @brief    
 * @author   David Hale <dhale@astro.caltech.edu>
 * @date     
 * @modified 
 *
 */

#ifndef EMULATORD_TCS_H
#define EMULATORD_TCS_H

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <algorithm>
#include <csignal>
#include <map>
#include <mutex>
#include <thread>
#include <vector>
#include <atomic>
#include <climits>

#include "config.h"
#include "network.h"
#include "common.h"
#include "tcs.h"
#include "tcsd_commands.h"

#define  BUFSIZE      1024  ///< size of the input command buffer

/***** TcsEmulator ************************************************************/
/**
 * @namespace TcsEmulator
 * @brief     this namespace contains everything for the TCS emulator
 *
 */
namespace TcsEmulator {


  /***** TcsEmulator::Server **************************************************/
  /**
   * @class  Server
   * @brief  TCS emulator server class
   *
   * This class contains everything needed to run the emulator server,
   * the server which responds as the TCS.
   *
   */
  class Server {
    private:
    public:
      int port;
      std::string subsystem;             ///< subsystem name
      std::atomic<int> cmd_num;
      Config config;
      std::mutex conn_mutex;             ///< mutex to protect against simultaneous access to Accept()

      TcsEmulator::Interface interface;  ///< create an emulater interface

      /***** TcsEmulator::Server **********************************************/
      /**
       * @fn         Server
       * @brief      class constructor
       * @param[in]  none
       * @return     none
       */
      Server() {
        this->port=-1;
        this->subsystem="tcs";
        this->cmd_num=0;
      }
      /***** TcsEmulator::Server **********************************************/


      /***** TcsEmulator::~Server *********************************************/
      /**
       * @fn         ~Server
       * @brief      class deconstructor cleans up on exit
       * @param[in]  none
       * @return     none
       */
      ~Server() {
      }
      /***** TcsEmulator::~Server *********************************************/


      /***** TcsEmulator::Server::exit_cleanly ********************************/
      /**
       * @fn         exit_cleanly
       * @brief      closes things nicely and exits
       * @param[in]  none
       * @return     none
       *
       */
      void exit_cleanly(void) {
        std::string function = "  (TcsEmulator::Server::exit_cleanly) ";
        std::cerr << get_timestamp() << function << "emulatord." << this->subsystem << " exiting\n";

        // close connection
        //
        if ( this->port > 0 ) close( this->port );
        exit( EXIT_SUCCESS );
      }
      /***** TcsEmulator::Server::exit_cleanly ********************************/


      /***** TcsEmulator::Server::load_tcs_info *******************************/
      /**
       * @brief      load tcs host info from config file into the class
       * @param[in]  input  input string expected to contain "name host port"
       * @return     -1 or port number
       *
       * This is a copy/paste from TCS::Server::load_tcs_info
       * but the only thing that is needed for the emulator is the port
       * number for the TCS named "sim" so this returns either the sim
       * port number or -1.
       *
       */
      long load_tcs_info( std::string input ) {
        std::string function = "  (TcsEmulator::Server::load_tcs_info) ";
        std::stringstream message;
        std::vector<std::string> tokens;
        std::string _name, _host;
        int _port=-1;

        // Extract the name, host and port from the input string
        //
        Tokenize( input, tokens, " \"" );

        if ( tokens.size() != 3 ) {
          std::cerr << get_timestamp() << function << "ERROR bad number of parameters in \"" << input 
                                                   << "\": expected 3 but received " << tokens.size() << "\n";
          return( -1 );
        }

        try {
          _name = tokens.at(0);
          _host = tokens.at(1);
          _port = std::stoi( tokens.at(2) );
        }
        catch ( std::invalid_argument &e ) {
          std::cerr << get_timestamp() << function << "ERROR loading tokens from input: " << input << ": " << e.what() << "\n";
          return( -1 );
        }
        catch ( std::out_of_range &e ) {
          std::cerr << get_timestamp() << function << "ERROR loading tokens from input: " << input << ": " << e.what() << "\n";
          return( -1 );
        }

        // Check that (potentially) valid values have been extracted
        //
        if ( _port < 1 ) {
          std::cerr << get_timestamp() << function << "ERROR port " << _port << " must be greater than 0\n";
          return( -1 );
        }
        if ( _name.empty() ) {
          std::cerr << get_timestamp() << function << "ERROR name cannot be empty\n";
          return( -1 );
        }
        if ( _host.empty() ) {
          std::cerr << get_timestamp() << function << "ERROR host cannot be empty\n";
          return( -1 );
        }

        // Return the port number if the name is sim
        //
        if ( _name.compare( "sim" ) == 0 ) return _port; else return -1;
      }
      /***** TcsEmulator::Server::load_tcs_info *******************************/


      /***** TcsEmulator::Server::configure_emulator **************************/
      /**
       * @fn         configure_emulator
       * @brief      
       * @param[in]  none
       * @return     ERROR or NO_ERROR
       *
       */
      long configure_emulator() {
        std::string function = "  (TcsEmulator::Server::configure_emulator) ";
        std::stringstream message;
        int applied=0;
        long error;

        // loop through the entries in the configuration file, stored in config class
        //
        for ( int entry=0; entry < this->config.n_entries; entry++ ) {

          try {
            // TCS_HOST
            if ( config.param[entry].compare( 0, 8, "TCS_HOST" ) == 0 ) {
              int _port = this->load_tcs_info( config.arg[entry] );
              if ( _port > 0 ) this->port = _port;
              applied++;
            }

            // EMULATOR_FOCUSRATE
            if ( config.param[entry].compare( 0, 18, "EMULATOR_FOCUSRATE" ) == 0 ) {
              this->interface.telescope.focusrate = std::stof( config.arg[entry] );
              applied++;
            }

            // EMULATOR_SLEWRATE_RA
            if ( config.param[entry].compare( 0, 20, "EMULATOR_SLEWRATE_RA" ) == 0 ) {
              this->interface.telescope.slewrate_ra = std::stof( config.arg[entry] );
              applied++;
            }

            // EMULATOR_SLEWRATE_DEC
            if ( config.param[entry].compare( 0, 21, "EMULATOR_SLEWRATE_DEC" ) == 0 ) {
              this->interface.telescope.slewrate_dec = std::stof( config.arg[entry] );
              applied++;
            }

            // EMULATOR_SLEWRATE_CASANGLE
            if ( config.param[entry].compare( 0, 26, "EMULATOR_SLEWRATE_CASANGLE" ) == 0 ) {
              this->interface.telescope.slewrate_casangle = std::stof( config.arg[entry] );
              applied++;
            }

            // EMULATOR_SETTLE_RA
            if ( config.param[entry].compare( 0, 18, "EMULATOR_SETTLE_RA" ) == 0 ) {
              this->interface.telescope.settle_ra = std::stof( config.arg[entry] );
              applied++;
            }

            // EMULATOR_SETTLE_DEC
            if ( config.param[entry].compare( 0, 19, "EMULATOR_SETTLE_DEC" ) == 0 ) {
              this->interface.telescope.settle_dec = std::stof( config.arg[entry] );
              applied++;
            }

            // EMULATOR_SETTLE_CASANGLE
            if ( config.param[entry].compare( 0, 24, "EMULATOR_SETTLE_CASANGLE" ) == 0 ) {
              this->interface.telescope.settle_casangle = std::stof( config.arg[entry] );
              applied++;
            }
          }
          catch ( std::invalid_argument &e ) {
            std::cerr << get_timestamp() << function << "ERROR interpreting string as number for " << config.arg[entry] << "\n";
            return( ERROR );
          }

        } // end loop through the entries in the configuration file

        std::cerr << get_timestamp() << function ;

        if ( applied==0 ) {
          std::cerr << "ERROR: " ;
          error = ERROR;
        } 
        else {
          error = NO_ERROR;
        } 
        std::cerr << "applied " << applied << " configuration lines to emulatord." << this->subsystem << "\n";

        return error;
      }
      /***** TcsEmulator::Server::configure_emulator **************************/

  };
  /***** TcsEmulator::Server **************************************************/

}
/***** TcsEmulator ************************************************************/
#endif
