/** ---------------------------------------------------------------------------
 * @file     acam_server.h
 * @brief    acam include file for the acquisition and guide camera
 * @author   David Hale <dhale@astro.caltech.edu>
 *
 * This file defines the Acam namespace and the main classes and functions
 * used by the acam server.
 *
 */

#ifndef ACAM_SERVER_H
#define ACAM_SERVER_H

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
#include "acam_interface.h"
#include "acamd_commands.h"


/***** Acam *******************************************************************/
/**
 * @namespace Acam
 * @brief     namespace for acquisition and guide camera (acam)
 *
 */
namespace Acam {

  const std::string DAEMON_NAME = "acamd";       ///< when run as a daemon, this is my name

  const int N_THREADS = 10;

  /***** Acam::Server *********************************************************/
  /**
   * @class  Server
   * @brief  server class for acam daemon
   *
   * This class defines what is needed by the acam daemon server.
   *
   */
  class Server {
    private:
    public:

      static Server* instance;

      /***** Acam::Server::Server *********************************************/
      /**
       * @brief      class constructor
       *
       */
      Server()
        : nbport(-1), blkport(-1), asyncport(-1), cmd_num(0), threads_active(0), id_pool(Acam::N_THREADS) {

        instance=this;

        // Register these signals
        //
        signal( SIGINT,  signal_handler );
        signal( SIGPIPE, signal_handler );
        signal( SIGHUP,  signal_handler );
      }
      /***** Acam::Server::Server *********************************************/


      /***** Acam::Server::~Server ********************************************/
      /**
       * @brief      class deconstructor cleans up on exit
       *
       */
      ~Server() {
        close_log();  // close the logfile, if open
      }
      /***** Acam::Server::~Server ********************************************/


      int nbport;                        ///< non-blocking port
      int blkport;                       ///< blocking port
      int asyncport;                     ///< asynchronous message port
      std::string asyncgroup;            ///< asynchronous multicast group

      std::atomic<int> cmd_num;          ///< keep a running tally of number of commands received by acamd
      std::atomic<int> threads_active;   ///< number of blocking threads that exist

      Config config;                     ///< create a Config object for reading the configuration file

      Interface interface;               ///< create an Interface object for the acam hardware

      NumberPool id_pool;                ///< creates a number pool to keep track of blocking socket IDs

      std::mutex conn_mutex;             ///< mutex to protect against simultaneous access to Accept()
      std::mutex sock_block_mutex;       ///< mutex to protect against simultaneous access to Accept()
      std::mutex socklist_mutex;         ///< mutex to protect against simultaneous access to socklist

      std::map<int, std::shared_ptr<Network::TcpSocket>> socklist;             ///< container to hold TcpSocket object pointers

      static void new_log_day( std::string logpath );                          ///< creates a new logbook each day
      static void block_main( Acam::Server &server, std::shared_ptr<Network::TcpSocket> sock );   ///< main function for blocking connection thread
      static void thread_main( Acam::Server &server, std::shared_ptr<Network::TcpSocket> sock );
      static void async_main( Acam::Server &acam, Network::UdpSocket sock );   ///< asynchronous message sending thread

      void doit(Network::TcpSocket sock);                                      ///< the workhorse of each thread connetion
      void exit_cleanly();                                                     ///< exit
      long configure_acamd();                                                  ///< read and apply the configuration file
      long configure_telemetry();                                              ///< read and apply telem configuration

      inline long initialize_python_objects() {                                ///< allows for initializing Python objects by the child process
        return( this->interface.initialize_python_objects() );
      }

      void handle_signal( int signo );

      static inline void signal_handler( int signo ) {
        if ( instance ) { instance->handle_signal( signo ); }
        return;
      }
  };
  /***** Acam::Server *********************************************************/

}
/***** Acam *******************************************************************/
#endif
