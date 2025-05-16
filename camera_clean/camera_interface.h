/**
 * @file    camera_interface.h
 * @brief   this defines the Camera::Interface base class
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#pragma once

#include "common.h"
#include "camera_information.h"
#include "camerad_commands.h"

namespace Camera {

  const std::string DAEMON_NAME = "camerad";

  class Server;  // forward declaration for Interface class

  class Interface {
    protected:
      Camera::Server* server=nullptr;
      Camera::Information camera_info;
      Common::FitsKeys systemkeys;

      // vector of pointers to Camera Information containers, one for each exposure number
      std::vector<std::shared_ptr<Camera::Information>> fitsinfo;

    public:
      virtual ~Interface() = default;

      Common::Queue async;  /// message queue object

      // These functions are shared by all interfaces with common implementations,
      // and are implemented in camera_interface.cpp
      //
      void handle_queue(std::string message);
      void set_server(Camera::Server* s);
      void func_shared();
      void disconnect_controller();

      // These virtual functions have interface-specific implementations
      // and must be implemented by derived classes, implemented in xxxx_interface.cpp
      //
      virtual long abort( std::string args, std::string &retstring ) = 0;
      virtual long autodir( std::string args, std::string &retstring ) = 0;
      virtual long basename( std::string args, std::string &retstring ) = 0;
      virtual long bias( std::string args, std::string &retstring ) = 0;
      virtual long bin( std::string args, std::string &retstring ) = 0;
      virtual long connect_controller( std::string args, std::string &retstring ) = 0;
      virtual long parse_controller_config(std::string args) = 0;
      virtual long configure_camera() = 0;
      virtual long disconnect_controller( std::string args, std::string &retstring ) = 0;
      virtual long exptime( std::string args, std::string &retstring ) = 0;
      virtual long expose( std::string args, std::string &retstring ) = 0;
      virtual long load_firmware( std::string args, std::string &retstring ) = 0;
      virtual long native( std::string args, std::string &retstring ) = 0;
      virtual long power( std::string args, std::string &retstring ) = 0;
      virtual long test( std::string args, std::string &retstring ) = 0;

  };

}
