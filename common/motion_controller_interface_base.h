/**
 * @file    motion_controller_interface_base.h
 * @brief   polymorphic base class
 * @details This defines the interface supported by all motion components
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#pragma once

#include <string>

/***** MotionController *******************************************************/
/**
 * @namespace MotionController
 * @brief     common classes and utilities for motion controllers
 *
 */
namespace MotionController {

  class AxisInfo;
  class PosInfo;

  /***** MotionController::ControllerInterface ********************************/
  /**
   * @brief    This defines the interface supported by all motion components
   * @details  This class contains pure virtual functions that must be
   *           overridden and implemented by a derived class. This is the
   *           public API that callers use.
   *
   */
  class ControllerInterface {
    public:
      ControllerInterface() = default;
      virtual ~ControllerInterface() noexcept = default;

      // there are name-based functions
      //
      virtual const AxisInfo* get_axis(const std::string &name, int axis) const = 0;
      virtual const PosInfo* get_posmap(const std::string &name, const std::string &posname) const = 0;

      virtual long add_posmap(const std::string &name, const PosInfo &posinfo) = 0;
      virtual long add_axis(const std::string &name, const AxisInfo &axis) = 0;

      virtual int get_naxes(const std::string &name) const = 0;
      virtual int get_addr(const std::string &name) const = 0;
      virtual int get_port(const std::string &name) const = 0;
      virtual std::string get_host(const std::string &name) const = 0;

      virtual long open(const std::string &name) = 0;
      virtual long close(const std::string &name) = 0;
      virtual long stop(const std::string &name) = 0;
      virtual long home(const std::string &name, std::string* retstring=nullptr) = 0;
      virtual bool is_home(const std::string &name) = 0;
      virtual bool is_connected(const std::string &name) = 0;
      virtual long enable_motion(const std::string &name, bool shouldenable) = 0;

      virtual long moveto(const std::string &name, int axisnum, const std::string &posstr, std::string &retstring) = 0;
      virtual long get_pos(const std::string &name, int axisnum, float &position,
                           std::string* posname=nullptr, int addr=-1) = 0;

      virtual long send_command(const std::string &name, const std::string &cmd, std::string* retstring=nullptr) = 0;

      // these are universal functions (not name-based)
      //
      /***
      virtual long load_controller_config(const std::string &input) = 0;
      virtual void clear_motormap() = 0;
      virtual long clear_errors() = 0;

      virtual long open() = 0;
      virtual long close() = 0;

      virtual long enable_motion(bool shouldenable) = 0;
      virtual long move_to_default() = 0;
      ***/
  };
  /***** MotionController::ControllerInterface ********************************/

}
/***** MotionController *******************************************************/
