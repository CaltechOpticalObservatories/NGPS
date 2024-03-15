/** ---------------------------------------------------------------------------
 * @file     focus_server.h
 * @brief    defines the namespace and server class for the slid daemon
 * @author   David Hale <dhale@astro.caltech.edu>
 *
 */

#ifndef FOCUS_SERVER_H
#define FOCUS_SERVER_H

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
#include "focus_interface.h"
#include "focusd_commands.h"


/***** Focus ******************************************************************/
/**
 * @namespace Focus
 * @brief     namespace for the focus system
 *
 */
namespace Focus {

  /***** Focus::Server ********************************************************/
  /**
   * @class Server
   * @brief creates a TCP/IP server for the focus daemon
   */
  class Server {
    private:
    public:
      /***** Focus::Server::Server ********************************************/
      /**
       * @brief  class constructor
       */
      Server() {
        this->nbport=-1;
        this->blkport=-1;
        this->asyncport=-1;
        this->cmd_num=0;
      }

      /***** Focus::Server::~Server *******************************************/
      /**
       * @brief  class deconstructor cleans up on exit
       */
      ~Server() {
        close(this->nonblocking_socket);
        close(this->blocking_socket);
        close_log();  // close the logfile, if open
      }
      /***** Focus::Server::~Server *******************************************/

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
      static void block_main( Focus::Server &tcs, Network::TcpSocket sock );   ///< main function for blocking connection thread
      static void thread_main( Focus::Server &tcs, Network::TcpSocket sock );  ///< main function for all non-blocked threads
      static void async_main( Focus::Server &tcs, Network::UdpSocket sock );   ///< asynchronous message sending thread

      void exit_cleanly(void);              ///< exit
      long configure_focusd();              ///< read and apply the configuration file
      void doit(Network::TcpSocket sock);   ///< the workhorse of each thread connetion

  };
  /***** Focus::Server ********************************************************/

}
/***** Focus ******************************************************************/
#endif
