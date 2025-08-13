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

  const std::string DAEMON_NAME = "camerad";  /// my name

  constexpr long MAX_SHUTTER_DELAY = 3000;    /// maximum shutter delay in msec

  class Server;                               /// forward declaration for Interface class

  class Interface {
    protected:
      Camera::Server* server=nullptr;
      Camera::Information camera_info;
      Common::FitsKeys systemkeys;

      // vector of pointers to Camera Information containers, one for each exposure number
      std::vector<std::shared_ptr<Camera::Information>> fitsinfo;

      mode_t dirmode;       /// user specified mode to OR with 0700 for imdir creation
      long shutter_delay;

    public:
      Interface() :
        shutter_delay(0) { }
      virtual ~Interface() = default;

      Common::Queue async;  /// message queue object

      // These functions are shared by all interfaces with common implementations,
      // and are implemented in camera_interface.cpp
      //
      void handle_queue(std::string message);
      void set_server(Camera::Server* s);
      void func_shared();
      void set_dirmode(mode_t mode);
      void disconnect_controller();
      long imdir( std::string args, std::string &retstring );
      long basename( std::string args, std::string &retstring );
      long set_shutter_delay( const std::string arg );
      long set_shutter_delay( long arg );

      // These virtual functions have interface-specific implementations
      // and must be implemented by derived classes, implemented in xxxx_interface.cpp
      //
      virtual long abort( std::string args, std::string &retstring ) = 0;
      virtual long autodir( std::string args, std::string &retstring ) = 0;
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
      virtual int devnum_from_chan( const std::string &chan ) = 0;
      virtual long test( std::string args, std::string &retstring ) = 0;

  };

}
