/** ---------------------------------------------------------------------------
 * @file     tcs_server.h
 * @brief    defines the namespace and server class for the tcs daemon
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
#include <json.hpp>

#include "logentry.h"
#include "config.h"
#include "network.h"
#include "common.h"
#include "tcs_interface.h"
#include "tcsd_commands.h"


/***** TCS ********************************************************************/
/**
 * @namespace TCS
 * @brief     namespace for the TCS daemon
 *
 */
namespace TCS {

  const int N_THREADS = 10;

  /***** TCS::Server **********************************************************/
  /**
   * @class  Server
   * @brief  server class for tcs daemon
   *
   * This class defines what is needed by the tcs daemon server.
   *
   */
  class Server {
    private:
    public:
      static Server* instance;

      /***** TCS::Server ******************************************************/
      /**
       * @brief  class constructor
       */
      Server() : nbport(-1), blkport(-1), asyncport(-1), cmd_num(0), threads_active(0), id_pool(TCS::N_THREADS) {
        instance=this;

        // Register these signals
        //
        signal( SIGINT,  signal_handler );
        signal( SIGPIPE, signal_handler );
        signal( SIGHUP,  signal_handler );
      }
      /***** TCS::Server ******************************************************/


      /***** TCS::~Server *****************************************************/
      /**
       * @brief      class deconstructor cleans up on exit
       *
       */
      ~Server() {
        close(this->nonblocking_socket);
        close(this->blocking_socket);
        close_log();  // close the logfile, if open
      }
      /***** TCS::~Server *****************************************************/


      int nbport;                        ///< non-blocking port
      int blkport;                       ///< blocking port
      int asyncport;                     ///< asynchronous message port
      std::atomic<int> cmd_num;          ///< keep a running tally of number of commands received by tcsd
      std::atomic<int> threads_active;   ///< number of blocking threads that exist

      std::string asyncgroup;            ///< asynchronous multicast group

      int nonblocking_socket;
      int blocking_socket;

      NumberPool id_pool;                   ///< creates a number pool

      Config config;                        ///< create a Config object for reading the configuration file

      Interface interface;

      std::mutex conn_mutex;                ///< mutex to protect against simultaneous access to Accept()
      std::mutex sock_block_mutex;          ///< mutex to protect against simultaneous access to Accept()
      std::mutex socklist_mutex;            ///< mutex to protect against simultaneous access to socklist

      std::map<int, std::shared_ptr<Network::TcpSocket>> socklist;              // create container to hold TcpSocket object pointers

      void remove_socket( int id  ) {
        std::lock_guard<std::mutex> lock( socklist_mutex );
        auto it = socklist.find( id );
        if ( it != socklist.end() ) {
          socklist.erase( it );
        }
        return;
      }

      static void new_log_day( std::string logpath );                        ///< creates a new logbook each day
      static void block_main( TCS::Server &tcs, std::shared_ptr<Network::TcpSocket> sock );   ///< main function for blocking connection thread
      static void thread_main( TCS::Server &tcs, std::shared_ptr<Network::TcpSocket> sock );
      static void handle_new_connection( TCS::Server &tcs, std::shared_ptr<Network::TcpSocket> sock );
      static void async_main( TCS::Server &tcs, Network::UdpSocket sock );   ///< asynchronous message sending thread

      void exit_cleanly(void);              ///< exit
      long load_tcs_info( std::string input );
      long configure_tcsd();                ///< read and apply the configuration file
      long configure_interface();           ///< read and apply the configuration file
      void doit(Network::TcpSocket &sock);   ///< the workhorse of each thread connetion

      void handle_signal( int signo );

      static inline void signal_handler( int signo ) {
        if ( instance ) { instance->handle_signal( signo ); }
        return;
      }
  };
  /***** TCS::Server **********************************************************/

}
/***** TCS ********************************************************************/
