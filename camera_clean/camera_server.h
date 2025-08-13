/**
 * @file     camera_server.h
 * @brief    
 * @author   David Hale <dhale@astro.caltech.edu>
 *
 */

#pragma once

//#include <map>
//#include <memory>
//#include <atomic>
//#include <mutex>
#include <limits.h>
#include <signal.h>
//#include <json.hpp>

#include "common.h"
#include "utilities.h"
#include "network.h"
#include "config.h"
#include "camerad_commands.h"

namespace Camera {

  const int N_THREADS=10;

  class Interface;  // forward declaration

  class Server {
    public:
      Server();
      ~Server();

      Interface* interface;

      NumberPool id_pool;
      Config config;

      int nbport;                        //!< non-blocking port
      int blkport;                       //!< blocking port
      int asyncport;                     //!< asynchronous message port
      std::string asyncgroup;            //!< asynchronous multicast group

      std::map<int, std::shared_ptr<Network::TcpSocket>> socklist;
      std::mutex sock_block_mutex;
      std::atomic<int> threads_active;
      std::atomic<int> cmd_num;

      long configure_server();
      long configure_interface();
      void handle_signal(int signo);
      void new_log_day(std::string logpath);
      void exit_cleanly();
      void block_main(std::shared_ptr<Network::TcpSocket> socket);
      void doit(Network::TcpSocket sock);
  };
}

