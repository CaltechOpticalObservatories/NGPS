/** ---------------------------------------------------------------------------
 * @file     telem_server.h
 * @brief    telemetry include file for the acquisition and guide camera
 * @author   David Hale <dhale@astro.caltech.edu>
 *
 * This file defines the Telemetry namespace and the main classes and functions
 * used by the telem server.
 *
 */

#ifndef TELEM_SERVER_H
#define TELEM_SERVER_H

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

#include "logentry.h"
#include "config.h"
#include "network.h"
#include "common.h"
#include "telem_interface.h"
#include "telemd_commands.h"


/***** Telemetry **************************************************************/
/**
 * @namespace Telemetry
 * @brief     namespace for acquisition and guide camera (telem)
 *
 */
namespace Telemetry {

  /***** Telemetry::Server ****************************************************/
  /**
   * @class  Server
   * @brief  server class for telemetry daemon
   *
   * This class defines what is needed by the telemetry daemon server.
   *
   */
  class Server {
    private:
    public:

      /***** Telemetry::Server::Server ****************************************/
      /**
       * @brief      class constructor
       *
       */
      Server() {
        this->nbport=-1;
        this->blkport=-1;
        this->asyncport=-1;
        this->cmd_num=0;
      }
      /***** Telemetry::Server::Server ****************************************/


      /***** Telemetry::Server::~Server ***************************************/
      /**
       * @brief      class deconstructor cleans up on exit
       *
       */
      ~Server() {
        close_log();  // close the logfile, if open
      }
      /***** Telemetry::Server::~Server ***************************************/


      int nbport;                        ///< non-blocking port
      int blkport;                       ///< blocking port
      int asyncport;                     ///< asynchronous message port
      std::string asyncgroup;            ///< asynchronous multicast group

      std::atomic<int> cmd_num;          ///< keep a running tally of number of commands received by telemd

      Config config;                     ///< create a Config object for reading the configuration file

      Interface interface;               ///< create an Interface object for the telem hardware

      std::mutex conn_mutex;             ///< mutex to protect against simultaneous access to Accept()

      static void new_log_day( std::string logpath );                          ///< creates a new logbook each day
      static void block_main( Telemetry::Server &telem, Network::TcpSocket sock );   ///< main function for blocking connection thread
      static void thread_main( Telemetry::Server &telem, Network::TcpSocket sock );  ///< main function for all non-blocked threads
      static void async_main( Telemetry::Server &telem, Network::UdpSocket sock );   ///< asynchronous message sending thread

      void doit(Network::TcpSocket sock);                                      ///< the workhorse of each thread connetion
      void exit_cleanly();                                                     ///< exit
      long configure_telemd();                                                 ///< read and apply the configuration file
  };
  /***** Telemetry::Server ****************************************************/

}
/***** Telemetry *******************************************************************/
#endif
