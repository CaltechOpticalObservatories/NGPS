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

      Config config;                     ///< create a Config object for reading the configuration file

      Interface interface;               ///< create an Interface object for the acam hardware

      std::mutex conn_mutex;             ///< mutex to protect against simultaneous access to Accept()

      static void new_log_day( std::string logpath );                          ///< creates a new logbook each day
      static void block_main( Acam::Server &acam, Network::TcpSocket sock );   ///< main function for blocking connection thread
      static void thread_main( Acam::Server &acam, Network::TcpSocket sock );  ///< main function for all non-blocked threads
      static void async_main( Acam::Server &acam, Network::UdpSocket sock );   ///< asynchronous message sending thread

      void doit(Network::TcpSocket sock);                                      ///< the workhorse of each thread connetion
      void exit_cleanly();                                                     ///< exit
      long configure_acamd();                                                  ///< read and apply the configuration file

      inline void initialize_python_objects() {                                ///< allows for initializing Python objects by the child process
        this->interface.initialize_python_objects();
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
