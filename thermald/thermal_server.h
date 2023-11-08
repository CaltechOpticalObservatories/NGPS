/** ---------------------------------------------------------------------------
 * @file     thermal_server.h
 * @brief    defines the namespace and server class for the slid daemon
 * @author   David Hale <dhale@astro.caltech.edu>
 *
 */

#ifndef THERMAL_SERVER_H
#define THERMAL_SERVER_H

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
#include <condition_variable>

#include "logentry.h"
#include "config.h"
#include "network.h"
#include "common.h"
#include "thermal_interface.h"
#include "thermald_commands.h"
#include "database.h"


/***** Thermal ****************************************************************/
/**
 * @namespace Thermal
 * @brief     namespace for the temperature controllers
 *
 */
namespace Thermal {

  const std::string DAEMON_NAME = "thermald";    ///< when run as a daemon, this is my name

  /***** Thermal::Server ******************************************************/
  /**
   * @class Server
   * @brief creates a TCP/IP server for the thermal daemon
   */
  class Server {
    private:
    public:
      /***** Thermal::Server::Server ******************************************/
      /**
       * @brief  class constructor
       */
      Server() {
        this->nbport=-1;
        this->blkport=-1;
        this->asyncport=-1;
        this->cmd_num=0;
      }

      /***** Thermal::Server::~Server *****************************************/
      /**
       * @brief  class deconstructor cleans up on exit
       */
      ~Server() {
        close(this->nonblocking_socket);
        close(this->blocking_socket);
        close_log();  // close the logfile, if open
      }
      /***** Thermal::Server::~Server *****************************************/

      int nbport;                           ///< non-blocking port
      int blkport;                          ///< blocking port
      int asyncport;                        ///< asynchronous message port
      std::string asyncgroup;               ///< asynchronous multicast group

      int nonblocking_socket;
      int blocking_socket;

      std::atomic<int> cmd_num;

      Config config;

      Interface interface;                  ///< the Interface class connects to the hardware

      int telem_period;
      InterruptableSleepTimer telem_sleeptimer;                    ///< allows interrupting telemetry
      long telemetry( std::string args, std::string &retstring );  ///< utility to stop/start/check status of telemetry
      std::vector<std::string> db_info;                 ///< info for constructing telemetry Database object

      std::atomic<bool> telem_running;      ///< is the main telemetry thread running?

      std::mutex conn_mutex;                ///< mutex to protect against simultaneous access to Accept()

      static void new_log_day( std::string logpath );                          ///< creates a new logbook each day
      static void block_main( Thermal::Server &tcs, Network::TcpSocket sock ); ///< main function for blocking connection thread
      static void thread_main( Thermal::Server &tcs, Network::TcpSocket sock );///< main function for all non-blocked threads
      static void async_main( Thermal::Server &tcs, Network::UdpSocket sock ); ///< asynchronous message sending thread
      static void telemetry_watchdog( Thermal::Server &server );               ///< keeps the main telemetry thread running
      static void dothread_telemetry( Thermal::Server &server );               ///< main telemetry thread

      void exit_cleanly(void);              ///< exit
      long configure_thermald();            ///< read and apply daemon config from configuration file
      long configure_lakeshore();           ///< read and apply Lakeshore config from configuration file
      long configure_telemetry();           ///< read and apply telemetry config from configuration file
      long parse_lks_unit( std::string &input, 
                           int &lksnum, std::string &model, std::string &name, std::string &host, int &port );
      long parse_lks_chan( std::string &input,
                           int &lksnum, std::string &chan, bool &heater, std::string &label );
      void doit(Network::TcpSocket sock);   ///< the workhorse of each thread connetion

  };
  /***** Thermal::Server ******************************************************/

}
/***** Thermal ****************************************************************/
#endif
