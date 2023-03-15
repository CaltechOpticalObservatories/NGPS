/** ---------------------------------------------------------------------------
 * @file     calib_server.h
 * @brief    defines the namespace and server class for the calib daemon
 * @author   David Hale <dhale@astro.caltech.edu>
 *
 */

#ifndef CALIB_SERVER_H
#define CALIB_SERVER_H

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
#include "calib_interface.h"
#include "calibd_commands.h"


/***** Calib ******************************************************************/
/**
 * @namespace Calib
 * @brief     namespace for the calibrator
 *
 */
namespace Calib {

  const std::string DAEMON_NAME = "calibd";      ///< when run as a daemon, this is my name

  /***** Calib::Server ********************************************************/
  /**
   * @class Server
   * @brief creates a TCP/IP server for the calib daemon
   */
  class Server {
    private:
    public:
      /***** Calib::Server::Server ********************************************/
      /**
       * @brief  class constructor
       */
      Server() {
        this->nbport=-1;
        this->blkport=-1;
        this->asyncport=-1;
        this->cmd_num=0;
      }

      /***** Calib::Server::~Server *******************************************/
      /**
       * @brief  class deconstructor cleans up on exit
       */
      ~Server() {
        close(this->nonblocking_socket);
        close(this->blocking_socket);
        close_log();  // close the logfile, if open
      }
      /***** Calib::Server::~Server *******************************************/

      int nbport;                           ///< non-blocking port
      int blkport;                          ///< blocking port
      int asyncport;                        ///< asynchronous message port
      std::string asyncgroup;               ///< asynchronous multicast group

      int nonblocking_socket;
      int blocking_socket;

      std::atomic<int> cmd_num;

      Config config;

      Interface interface;                  ///< the Interface class connects to the hardware

      std::mutex conn_mutex;                ///< mutex to protect against simultaneous access to Accept()

      static void new_log_day( std::string logpath );                          ///< creates a new logbook each day
      static void block_main( Calib::Server &tcs, Network::TcpSocket sock );   ///< main function for blocking connection thread
      static void thread_main( Calib::Server &tcs, Network::TcpSocket sock );  ///< main function for all non-blocked threads
      static void async_main( Calib::Server &tcs, Network::UdpSocket sock );   ///< asynchronous message sending thread

      void exit_cleanly(void);              ///< exit
      long configure_calibd();              ///< read and apply the configuration file
      void doit(Network::TcpSocket sock);   ///< the workhorse of each thread connetion

  };
  /***** Calib::Server ********************************************************/

}
/***** Calib ******************************************************************/
#endif
