/**
 * @file    motion_controller.h
 * @brief   common classes and utilities for motion controllers
 * @details These implementations can be shared between PI and Galil
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#pragma once

#include "common.h"
#include "network.h"
#include "utilities.h"

#include <string>
#include <vector>
#include <mutex>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <map>

/***** MotionController *******************************************************/
/**
 * @namespace MotionController
 * @brief     common classes and utilities for motion controllers
 *
 */
namespace MotionController {

  /***** MotionController::AxisInfo *******************************************/
  /**
   * @class    AxisInfo
   * @brief    information for a motor axis
   * @details  This class contains motor axis info and function for parsing
   *           the MOTOR_AXIS key from a config file.
   *
   */
  class AxisInfo {
    public:
      std::string motorname;      ///< motor controller name
      int axisnum;                ///< 1-based axis number {1,2,3,...}
      float pos;                  ///< current position of this actuator
      bool ishome;                ///< is axis homed?
      bool ontarget;              ///< is axis on target?
      float min;                  ///< min travel range of motor connected to this controller
      float max;                  ///< max travel range of motor connected to this controller
      float zeropos;              ///< defined zero position of motor
      float defpos;               ///< optional default position to move after referencing (NaN if not specified)
      std::string reftype;        ///< reference type can be { neg, pos, ref }

      AxisInfo() : axisnum(0), pos(NAN), ishome(false), ontarget(false), min(NAN), max(NAN), zeropos(NAN) { }

      long load_axis_info(const std::string &input);
  };
  /***** MotionController::AxisInfo *******************************************/


  /***** MotionController::PosInfo ********************************************/
  /**
   * @class    PosInfo
   * @brief    information for position maps
   *
   */
  class PosInfo {
    public:
      std::string motorname;
      int axis;
      int posid;
      float position;
      std::string posname;

      long load_pos_info(const std::string &input);
  };
  /***** MotionController::PosInfo ********************************************/


  /***** MotionController::ControllerInfoBase *********************************/
  /**
   * @class    ControllerInfoBase
   * @brief    Base contoller information class
   * @details  This is the base class which contains members and functions
   *           common to all types of controllers. It is intended to be
   *           inherited by each specific class. This will always be
   *           constructed first.
   *
   */
  class ControllerInfoBase {
    public:
      std::string name;                       ///< controller name
      int addr;                               ///< controller address
      int naxes;
      std::map<int, AxisInfo> axes;           ///< info for each axis
      std::map<std::string, PosInfo> posmap;  ///< map of positions indexed by position name

      ControllerInfoBase() : name(""), addr(-1), naxes(-1) { }

  };
  /***** MotionController::ControllerInfoBase *********************************/


  /***** MotionController::ControllerInfo *************************************/
  /**
   * @class    ControllerInfo
   * @brief    Template class for all controller types
   * @details  This is a template class for types <T> where T is one of the
   *           derived classes representing a specific controller type. You
   *           can create a specialization by instantiating an object, E.G.,
   *           "ControllerInfo<StepperInfo> stepper" to specialize it for a
   *           stepper controller. This will always be constructed last,
   *           after the derived class.
   *
   */
  template <typename T>
  class ControllerInfo : public T {
    public:
      std::string host;                              ///< each motor controller defines its own host and port
      int port;

      ControllerInfo() : host(""), port(-1) { }      ///< constructor

      long load_controller_info(std::string input);
      long load_pos_info(std::string input);
  };
  /***** MotionController::ControllerInfo *************************************/


  /***** MotionController::Interface ******************************************/
  /**
   * @class  Interface
   * @brief  MotionController interface class
   *
   */
  template <typename ControllerType>
  class Interface {
    protected:
      bool is_initialized;
      std::string name;        ///< a name for info purposes
      std::unique_ptr<std::mutex> controller_mutex;
      int move_timeout;
      int home_timeout;
      float tolerance;
      volatile std::atomic<int> motors_running;
      volatile std::atomic<long> thread_error;

      // map of sockets allows use of multiple socket connections, indexed by {host,port}
      std::map<std::pair<std::string, int>, Network::TcpSocket> socketmap;

      // map of motors allows use of multiple motors, indexed by motor name
      std::map<std::string, ControllerInfo<ControllerType>> motormap;

    private:
      long _open(const std::string& motorname);

    public:

      // every controller must implement these functions
      //
      virtual void test() = 0;

      // these have common implementations
      //
      bool get_initialized() { return this->is_initialized; };
      std::map<std::string, ControllerInfo<ControllerType>> get_motormap() const { return motormap; }
      inline void clear_motormap() { motormap.clear(); }

      long open();
      long close();
      long close(const std::string &name);
      bool is_connected(const std::string &motorname);
      long add_posmap(const PosInfo &posinfo);
      long add_axis(const AxisInfo &axis);
      long load_controller_config(const std::string &input);

      Network::TcpSocket &get_socket(const std::string &motorname);

      // Default constructor
      //
      Interface()
        : is_initialized(false), name(""), controller_mutex(std::make_unique<std::mutex>()),
          move_timeout(60000), home_timeout(60000), tolerance(0.001),
          motors_running(0), thread_error(NO_ERROR) { }

      // Constructor initializes move and home timeouts
      //
      Interface(int TO_move, int TO_home, float tol)
        : is_initialized(false), name(""), controller_mutex(std::make_unique<std::mutex>()),
          move_timeout(TO_move), home_timeout(TO_home), tolerance(tol),
          motors_running(0), thread_error(NO_ERROR) { }

      // Copy constructor
      //
      Interface( const Interface &other )
        : is_initialized(other.is_initialized),
          name(other.name),
          controller_mutex(std::make_unique<std::mutex>()),
          move_timeout(other.move_timeout),
          home_timeout(other.home_timeout),
          tolerance(other.tolerance),
          motors_running(other.motors_running),
          thread_error(other.thread_error),
          socketmap(other.socketmap),
          motormap(other.motormap) {}

      // Copy assignment operator
      //
      Interface &operator = ( const Interface &other ) {
        if ( this != &other ) {
          is_initialized = other.is_initialized;
          name = other.name;
          controller_mutex = std::make_unique<std::mutex>();  // Create a new mutex for the copy
          move_timeout = other.move_timeout;
          home_timeout = other.home_timeout;
          tolerance = other.tolerance;
          motors_running = other.motors_running;
          thread_error = other.thread_error;
          socketmap = other.socketmap;
          motormap = other.motormap;
        }
        return *this;
      }

      // Move constructor
      //
      Interface( Interface &&other ) noexcept
        : is_initialized(other.is_initialized),
          name(std::move(other.name)),
          controller_mutex(std::move(other.controller_mutex)),
          move_timeout(std::move(other.move_timeout)),
          home_timeout(std::move(other.home_timeout)),
          tolerance(std::move(other.tolerance)),
          motors_running(std::move(other.motors_running)),
          thread_error(std::move(other.thread_error)),
          socketmap(std::move(other.socketmap)),
          motormap(std::move(other.motormap)) { other.is_initialized = false; }

      // Move assignment operator
      //
      Interface &operator = ( Interface &&other ) noexcept {
        if ( this != &other ) {
          is_initialized = other.is_initialized;
          name = std::move( other.name );
          controller_mutex = std::move( other.controller_mutex );
          move_timeout = std::move( other.move_timeout );
          home_timeout = std::move( other.home_timeout );
          tolerance = std::move( other.tolerance );
          motors_running = std::move( other.motors_running );
          thread_error = std::move( other.thread_error );
          socketmap = std::move( other.socketmap );
          motormap = std::move( other.motormap );
          other.is_initialized = false;
          other.motors_running = 0;
          other.thread_error = NO_ERROR;
        }
        return *this;
      }

  };
  /***** MotionController::Interface ******************************************/

}
#include "motion_controller_impl.h"  ///< implementations for ControllerInfo template
#include "motion_interface_impl.h"   ///< implementations for Interface template
/***** MotionController *******************************************************/
