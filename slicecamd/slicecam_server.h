/** ---------------------------------------------------------------------------
 * @file     slicecam_server.h
 * @brief    include file for the slicer cameras
 * @author   David Hale <dhale@astro.caltech.edu>
 *
 * This file defines the Slicecam namespace and the main classes and functions
 * used by the slicecam server.
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
#include "slicecam_interface.h"
#include "slicecamd_commands.h"


/***** Slicecam ***************************************************************/
/**
 * @namespace Slicecam
 * @brief     namespace for acquisition and guide camera (slicecam)
 *
 */
namespace Slicecam {

  const std::string DAEMON_NAME = "slicecamd";       ///< when run as a daemon, this is my name

  const int N_THREADS = 10;

  /***** Slicecam::Server *****************************************************/
  /**
   * @class  Server
   * @brief  server class for slicecam daemon
   *
   * This class defines what is needed by the slicecam daemon server.
   *
   */
  class Server {
    private:
    public:

      static Server* instance;

      /***** Slicecam::Server::Server *****************************************/
      /**
       * @brief      class constructor
       *
       */
      Server() {
        this->nbport=-1;
        this->blkport=-1;
        this->asyncport=-1;
        this->cmd_num.store( 0 );
        instance=this;

        // Register these signals
        //
        signal( SIGINT,  signal_handler );
        signal( SIGPIPE, signal_handler );
        signal( SIGHUP,  signal_handler );

      }
      /***** Slicecam::Server::Server *****************************************/


      /***** Slicecam::Server::~Server ****************************************/
      /**
       * @brief      class deconstructor cleans up on exit
       *
       */
      ~Server() {
        close_log();  // close the logfile, if open
      }
      /***** Slicecam::Server::~Server ****************************************/


      int nbport;                        ///< non-blocking port
      int blkport;                       ///< blocking port
      int asyncport;                     ///< asynchronous message port
      std::string asyncgroup;            ///< asynchronous multicast group

      std::atomic<int> cmd_num;          ///< keep a running tally of number of commands received by slicecamd

      Config config;                     ///< create a Config object for reading the configuration file

      Interface interface;               ///< create an Interface object for the slicecam hardware

      std::mutex conn_mutex;             ///< mutex to protect against simultaneous access to Accept()

      static void new_log_day( std::string logpath );                          ///< creates a new logbook each day
      static void block_main( Slicecam::Server &slicecam, Network::TcpSocket sock );   ///< main function for blocking connection thread
      static void thread_main( Slicecam::Server &slicecam, Network::TcpSocket sock );  ///< main function for all non-blocked threads
      static void async_main( Slicecam::Server &slicecam, Network::UdpSocket sock );   ///< asynchronous message sending thread

      void doit(Network::TcpSocket sock);                                      ///< the workhorse of each thread connetion
      void exit_cleanly();                                                     ///< exit
      long configure_slicecamd();                                                  ///< read and apply the configuration file

      void handle_signal( int signo );

      static inline void signal_handler( int signo ) {
        if ( instance ) { instance->handle_signal( signo ); }
        return;
      }
  };
  /***** Slicecam::Server *****************************************************/

}
/***** Slicecam ***************************************************************/
#endif
