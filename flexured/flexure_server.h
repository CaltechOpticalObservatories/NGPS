/** ---------------------------------------------------------------------------
 * @file     flexure_server.h
 * @brief    defines the namespace and server class for the slid daemon
 * @author   David Hale <dhale@astro.caltech.edu>
 *
 */

#pragma once

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
#include "flexure_interface.h"
#include "flexured_commands.h"


/***** Flexure ****************************************************************/
/**
 * @namespace Flexure
 * @brief     namespace for the flexure system
 *
 */
namespace Flexure {

  constexpr int N_THREADS = 10;

  /***** Flexure::Server ******************************************************/
  /**
   * @class Server
   * @brief creates a TCP/IP server for the flexure daemon
   */
  class Server {
    private:
    public:
      static Server* instance;

      /***** Flexure::Server::Server ******************************************/
      /**
       * @brief  class constructor
       */
      Server() : nbport(-1), blkport(-1), asyncport(-1), cmd_num(0), threads_active(0), id_pool(Flexure::N_THREADS) {
        instance=this;
        // register these signals
        //
        signal(SIGINT,  this->signal_handler);
        signal(SIGPIPE, this->signal_handler);
        signal(SIGHUP,  this->signal_handler);
      }

      /***** Flexure::Server::~Server *****************************************/
      /**
       * @brief  class deconstructor cleans up on exit
       */
      ~Server() {
        close(this->nonblocking_socket);
        close(this->blocking_socket);
        close_log();  // close the logfile, if open
      }
      /***** Flexure::Server::~Server *****************************************/

      int nbport;                           ///< non-blocking port
      int blkport;                          ///< blocking port
      int asyncport;                        ///< asynchronous message port
      std::string asyncgroup;               ///< asynchronous multicast group

      int nonblocking_socket;
      int blocking_socket;

      std::atomic<int> cmd_num;
      std::atomic<int> threads_active;      ///< number of blocking threads that exist

      NumberPool id_pool;                   ///< creates a number pool used for socket ids

      Config config;

      Interface interface;                  ///< the Interface class connects to the hardware

      std::mutex conn_mutex;                ///< mutex to protect against simultaneous access to Accept()
      std::mutex sock_block_mutex;          ///< mutex to protect against simultaneous access to Accept()
      std::mutex socklist_mutex;            ///< mutex to protect against simultaneous access to socklist

      std::map<int, std::shared_ptr<Network::TcpSocket>> socklist;             ///< container to hold TcpSocket object pointers

      void remove_socket( int id ) {                                           ///< removes a socket object from socklist
        std::lock_guard<std::mutex> lock( socklist_mutex );
        auto it = socklist.find( id );
        if ( it != socklist.end() ) socklist.erase( it );
        return;
      }

      static void new_log_day( std::string logpath );                          ///< creates a new logbook each day
      static void block_main( Flexure::Server &tcs, std::shared_ptr<Network::TcpSocket> sock ); ///< main function for blocking connection thread
      static void thread_main( Flexure::Server &tcs, std::shared_ptr<Network::TcpSocket> sock );  ///< main function for all non-blocked threads
      static void async_main( Flexure::Server &tcs, Network::UdpSocket sock );   ///< asynchronous message sending thread

      void exit_cleanly(void);              ///< exit
      long configure_flexured();            ///< read and apply the configuration file
      void doit(Network::TcpSocket &sock);  ///< the workhorse of each thread connetion

      void handle_signal(int signo);
      static inline void signal_handler(int signo) { if (instance) { instance->handle_signal(signo); } }
  };
  /***** Flexure::Server ******************************************************/

}
/***** Flexure ****************************************************************/
