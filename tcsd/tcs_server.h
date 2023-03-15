/** ---------------------------------------------------------------------------
 * @file     tcs_server.h
 * @brief    defines the namespace and server class for the tcs daemon
 * @author   David Hale <dhale@astro.caltech.edu>
 *
 */

#ifndef TCS_SERVER_H
#define TCS_SERVER_H

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
#include "tcs_interface.h"
#include "tcsd_commands.h"


/***** TCS ********************************************************************/
/**
 * @namespace TCS
 * @brief     namespace for the TCS daemon
 *
 */
namespace TCS {

  const std::string DAEMON_NAME = "tcsd";        ///< when run as a daemon, this is my name

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
      /***** TCS::Server ******************************************************/
      /**
       * @brief  class constructor
       */
      Server() {
        this->nbport=-1;
        this->blkport=-1;
        this->asyncport=-1;
        this->cmd_num=0;
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
      std::string asyncgroup;            ///< asynchronous multicast group

      int nonblocking_socket;
      int blocking_socket;

      std::atomic<int> cmd_num;             ///< keep a running tally of number of commands received by tcsd

      Config config;                        ///< create a Config object for reading the configuration file

      Interface interface;

      std::mutex conn_mutex;                ///< mutex to protect against simultaneous access to Accept()

      static void new_log_day( std::string logpath );                        ///< creates a new logbook each day
      static void block_main( TCS::Server &tcs, Network::TcpSocket sock );   ///< main function for blocking connection thread
      static void thread_main( TCS::Server &tcs, Network::TcpSocket sock );  ///< main function for all non-blocked threads
      static void async_main( TCS::Server &tcs, Network::UdpSocket sock );   ///< asynchronous message sending thread

      void exit_cleanly(void);              ///< exit
      long configure_tcsd();                ///< read and apply the configuration file
      void doit(Network::TcpSocket sock);   ///< the workhorse of each thread connetion

  };
  /***** TCS::Server **********************************************************/

}
/***** TCS ********************************************************************/
#endif
