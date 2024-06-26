/** ---------------------------------------------------------------------------
 * @file     sequencer_server.h
 * @brief    defines the namespace and server class for the observation sequencer
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
#include <condition_variable>
#include <cmath>
#include <ctgmath>

#include "logentry.h"
#include "config.h"
#include "network.h"
#include "common.h"
#include "sequencer_interface.h"
#include "sequence.h"
#include "sequencerd_commands.h"


/***** Sequencer **************************************************************/
/**
 * @namespace Sequencer
 * @brief     namespace for the observation sequencer
 *
 */
namespace Sequencer {

  const std::string DAEMON_NAME = "sequencerd";  ///< when run as a daemon, this is my name
  const std::string ACK = "ACK\n";
  const std::string ERR = "ERR\n";

  const int N_THREADS = 10;

  /***** Sequencer::Server ****************************************************/
  /**
   * @class Server
   * @brief server class for the sequencer daemon
   *
   * This class defines what is needed by the sequencer daemon server.
   *
   */
  class Server {
    private:
    public:

      static Server* instance;

      /***** Sequencer::Server ************************************************/
      /**
       * @brief  class constructor
       */
      Server() {
        this->nbport=-1;
        this->blkport=-1;
        this->asyncport=-1;
        this->messageport=-1;
        this->cmd_num.store(0);
        instance=this;

        // Register these signals
        //
        signal( SIGINT,  signal_handler );
        signal( SIGPIPE, signal_handler );
        signal( SIGHUP,  signal_handler );

        // The names of all of the daemons defined in the Sequencer::Sequence class
        // are initialized here. The names are useful just for logging.
        //
        this->sequence.calibd.name   = "calibd";
        this->sequence.camerad.name  = "camerad";
        this->sequence.filterd.name  = "filterd";
        this->sequence.flexured.name = "flexured";
        this->sequence.focusd.name   = "focusd";
        this->sequence.powerd.name   = "powerd";
        this->sequence.slitd.name    = "slitd";
        this->sequence.tcsd.name     = "tcsd";
      }
      /***** Sequencer::Server ************************************************/


      /***** Sequencer::~Server ***********************************************/
      /**
       * @brief  class deconstructor cleans up on exit
       */
      ~Server() {
        close(this->nonblocking_socket);
        close(this->blocking_socket);
        close_log();  // close the logfile, if open
      }
      /***** Sequencer::~Server ***********************************************/

      int nbport;                           ///< sequencer daemon non-blocking port
      int blkport;                          ///< sequencer daemon blocking port
      int asyncport;                        ///< sequencer daemon asynchronous command port
      int messageport;                      ///< sequencer daemon asynchronous message port
      std::string messagegroup;             ///< sequencer daemon asynchronous message multicast group

      int nonblocking_socket;
      int blocking_socket;

      std::atomic<int> cmd_num;             ///< keep a running tally of number of commands received by sequencerd

      Config config;                        ///< create a Config object for reading the configuration file

      Sequence sequence;                    ///< create a Sequence object--this is where the work is done

      std::mutex conn_mutex;                ///< mutex to protect against simultaneous access to Accept()

      void exit_cleanly(void);              ///< exit
      long configure_sequencer();           ///< read and apply the configuration file
      static void doit(Sequencer::Server &seq, Network::TcpSocket &sock);  ///< the workhorse of each thread connetion

      static void new_log_day( std::string logpath );                               ///< creates a new logbook each day
      static void block_main( Sequencer::Server &seq, Network::TcpSocket sock );    ///< main function for blocking connection thread
      static void thread_main( Sequencer::Server &seq, Network::TcpSocket &sock );  ///< main function for all non-blocked threads
      static void gui_main( Sequencer::Server &seq, Network::TcpSocket &sock );     ///< main function for gui threads
      static void async_main( Sequencer::Server &seq, Network::UdpSocket sock );    ///< asynchronous message sending thread

      void handle_signal( int signo );

      static inline void signal_handler( int signo ) {
        if ( instance ) { instance->handle_signal( signo ); }
        return;
      }

  };
  /***** Sequencer::Server ****************************************************/

}
/***** Sequencer **************************************************************/
