/** ---------------------------------------------------------------------------
 * @file     emulatord_power.h
 * @brief    
 * @author   David Hale <dhale@astro.caltech.edu>
 *
 */

#ifndef EMULATORD_POWER_H
#define EMULATORD_POWER_H

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
#include <regex>

#include "config.h"
#include "network.h"
#include "common.h"
#include "power.h"
#include "powerd_commands.h"

#define  BUFSIZE      1024  ///< size of the input command buffer

/***** PowerEmulator **********************************************************/
/**
 * @namespace PowerEmulator
 * @brief     this namespace contains everything for the power emulator
 *
 */
namespace PowerEmulator {


  /***** PowerEmulator::Server ************************************************/
  /**
   * @class  Server
   * @brief  emulator server class
   *
   * This class contains everything needed to run the emulator server,
   * the server which responds as the NPS.
   *
   */
  class Server {
    private:
    public:
      int port;
      std::string subsystem;                ///< subsystem name
      std::atomic<int> cmd_num;
      Config config;
      std::mutex conn_mutex;                ///< mutex to protect against simultaneous access to Accept()

      PowerEmulator::Interface interface;   ///< create an emulated interface

      /***** PowerEmulator::Server::Server ************************************/
      /**
       * @fn         Server
       * @brief      class constructor
       * @param[in]  none
       * @return     none
       */
      Server() {
        this->port=-1;
        this->subsystem="power";
        this->cmd_num=0;
      }
      /***** PowerEmulator::Server::Server ************************************/


      /***** PowerEmulator::Server::~Server ***********************************/
      /**
       * @fn         ~Server
       * @brief      class deconstructor cleans up on exit
       * @param[in]  none
       * @return     none
       */
      ~Server() {
      }
      /***** PowerEmulator::Server::~Server ***********************************/


      /***** PowerEmulator::Server::exit_cleanly ******************************/
      /**
       * @fn         exit_cleanly
       * @brief      closes things nicely and exits
       * @param[in]  none
       * @return     none
       *
       */
      void exit_cleanly(void) {
        std::string function = "  (PowerEmulator::Server::exit_cleanly) ";
        std::cerr << get_timestamp() << function << "emulatord." << this->subsystem << " exiting\n";

        // close connection
        //
        if ( this->port > 0 ) close( this->port );
        exit( EXIT_SUCCESS );
      }
      /***** PowerEmulator::Server::exit_cleanly ******************************/


      /***** PowerEmulator::Server::configure_emulator ************************/
      /**
       * @fn         configure_emulator
       * @brief      
       * @param[in]  none
       * @return     ERROR or NO_ERROR
       *
       */
      long configure_emulator() {
        std::string function = "  (PowerEmulator::Server::configure_emulator) ";
        int applied=0;
        long error = NO_ERROR;

        // loop through the entries in the configuration file, stored in config class
        //
        for ( int entry=0; entry < this->config.n_entries; entry++ ) {

          // NPS_UNIT
          if ( config.param[entry].compare( 0, 8, "NPS_UNIT" ) == 0 ) {
            // Create a local NpsInfo object for checking the config file input.
            //
            PowerEmulator::NpsInfo npsinfo;

            // The nps_info map is indexed by nps number.
            // Pass this variable by reference to the load_nps_info() function, which will
            // set it to a valid value for insertion into the nps_info map.
            //
            int npsnum=-1;

            if ( npsinfo.load_nps_info( config.arg[entry], npsnum ) == NO_ERROR ) {
              this->interface.nps_info.insert( { npsnum, npsinfo } );  // insert this into the nps_info map
              applied++;
            }
          }

          // NPS_PLUG
          if (config.param[entry].compare(0, 8, "NPS_PLUG")==0) {
            // Create a local NpsInfo object for checking the config file input.
            //
            PowerEmulator::NpsInfo npsinfo;

            // The following variables (plugname, npsnum, plugnum) are extracted from this config file
            // by the load_plug_info() function.
            //
            std::string plugname;
            int         npsnum;
            int         plugnum;
            int         state=0;   /// default off

            if ( npsinfo.load_plug_info( config.arg[entry], npsnum, plugnum, plugname ) == NO_ERROR ) {

              // load_plug_info() cannot check for maxplugs so check for that now
              //
              int maxplugs=-1;

              // Make sure the plug's npsnum has a matching NPS unit definition in the interface's nps_info map.
              //
              auto loc = this->interface.nps_info.find( npsnum );      // find this plug's npsnum in the nps_info map.

              if ( loc != this->interface.nps_info.end() ) {           // found
                maxplugs = loc->second.maxplugs;                       // here's the maxplugs for this unit
              }
              else {                                                   // else not found
                std::cerr << get_timestamp() << function << "ERROR plug " << plugnum << " " << plugname
                                             << " has no matching NPS unit number " << npsnum << " defined by NPS_UNIT";
                error = ERROR;
                break;
              }

              if ( plugnum > maxplugs ) {
                std::cerr << get_timestamp() << function << "ERROR bad plug number " << plugnum << " for " << plugname
                                             << " on unit " << npsnum << ": must be < " << maxplugs;
                error = ERROR;
                break;
              }

              // This plug passed all the tests so insert the plug into the plugstate map.
              // This map provides for locating the state by plugnum.
              //
              this->interface.nps_info.at(npsnum).plugstate.insert( { plugnum, state } );

              applied++;
            }
            else error = ERROR;
          }

        } // end loop through the entries in the configuration file

        std::cerr << get_timestamp() << function ;

        if ( applied==0 ) {
          std::cerr << "ERROR: " ;
          error = ERROR;
        } 
        std::cerr << "applied " << applied << " configuration lines to emulatord." << this->subsystem << "\n";

        return error;
      }
      /***** PowerEmulator::Server::configure_emulator ************************/

  };
  /***** PowerEmulator::Server ************************************************/

}
/***** PowerEmulator **********************************************************/
#endif
