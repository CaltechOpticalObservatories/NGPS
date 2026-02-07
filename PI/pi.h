/**
 * @file    pi.h
 * @brief   this file contains the definitions for the PI interface
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#pragma once

#include "motion_controller.h"
#include "common.h"
#include "network.h"
#include "utilities.h"
#include "pi_error.h"

#include <string>
#include <vector>
#include <mutex>

/***** Physik_Instrumente *****************************************************/
/**
 * @namespace Physik_Instrumente
 * @brief     namespace for Physik Instrumente hardware
 * @details   This namespace contains Interface and ControllerInfo classes.
 *            The Interface class contains the functions which communicate
 *            with the hardware. The ControllerInfo class contains the
 *            information from the configuration file.
 *
 */
namespace Physik_Instrumente {

  // Use common classes directly
  using AxisInfo = MotionController::AxisInfo;  ///< information for a motor axis
  using PosInfo = MotionController::PosInfo;    ///< information for position maps

  template <typename T>
  using ControllerInfo = MotionController::ControllerInfo<T>;

  /***** PiezoInfo ************************************************************/
  /**
   * @class    PiezoInfo
   * @brief    Derived class containing information specific to piezo controllers
   * @details  This is the derived class for piezo controllers, containing
   *           members relevant only to piezo controllers. It inherits the base
   *           class to gain access to common base members. This will always be
   *           constructed after the base class (and before the specialization).
   *
   */
  class PiezoInfo : public MotionController::ControllerInfoBase {
    public:

      PiezoInfo() { }                                ///< constructor

      // The following are special functions to load info from the config
      // file into the class, for this type of controller.
      //
      long load_controller_info( std::vector<std::string> tokens );
  };
  /***** PiezoInfo ************************************************************/


  /***** ServoInfo ************************************************************/
  /**
   * @class    ServoInfo
   * @brief    Derived class contains information specific to servo controllers
   * @details  This is the derived class for servo controllers, containing
   *           members relevant only to servo controllers. It inherits the base
   *           class to gain access to common base members. This will always be
   *           constructed after the base class (and before the specialization).
   *
   */
  class ServoInfo : public MotionController::ControllerInfoBase {
    public:
      bool servo;                 ///< servo state (true=on, false=off)

      ServoInfo() : servo(false) { }

      long load_controller_info( std::vector<std::string> tokens );
  };
  /***** ServoInfo ************************************************************/


  /***** StepperInfo **********************************************************/
  /**
   * @class    StepperInfo
   * @brief    Derived class containing information specific to stepper controllers
   * @details  This is the derived class for stepper controllers, containing
   *           members relevant only to stepper controllers. It inherits the base
   *           class to gain access to common base members. This will always be
   *           constructed after the base class (and before the specialization).
   *
   */
  class StepperInfo : public MotionController::ControllerInfoBase {
    public:

      StepperInfo() { }                              ///< constructor

      // The following are special functions to load info from the config
      // file into the class, for this type of controller.
      //
      long load_controller_info( std::vector<std::string> tokens );
  };
  /***** StepperInfo **********************************************************/


  /***** Interface ************************************************************/
  /**
   * @class  Interface
   * @brief  PI interface class
   *
   */
  template <typename ControllerType>
  class Interface : public MotionController::Interface<ControllerType> {
    protected:
      // these functions are for use within the class only
      //

      // moving
      //
      static void _dothread_moveto( Interface<ControllerType> &iface,
                                                   const std::string name,
                                                   const int addr,
                                                   const int axis,
                                                   const float pos );
      long _move_axis_wait( const std::string &name, int addr, int axis );
      long _move_abs( const std::string &name, int addr, float pos );
      long _move_abs( const std::string &name, int addr, int axis, float pos );
      long _move_rel( const std::string &name, int addr, float pos );
      long _move_rel( const std::string &name, int addr, int axis, float pos );

      // homing
      //
      static void _dothread_home( Interface<ControllerType> &iface, const std::string name );
      long _home_axis_wait( const std::string &name, int addr, int axis );
      long _home_axis( const std::string &name, int addr, std::string ref );
      long _home_axis( const std::string &name, int addr, int axis, std::string ref );

      long _get_pos( const std::string &name, int addr, float &pos );
      long _get_pos( const std::string &name, int addr, int axis, float &pos );

    public:
      using MotionController::Interface<ControllerType>::Interface;

      // functions defined in the base class must be implemented here
      //
      long move_to_default();
      long enable_motion(bool shouldenable);
      long enable_motion(const std::string &name, bool shouldenable);
      long stop(const std::string &name);

      // override/implement PI-specific default functions
      //
      void test() override;
      bool is_home(const std::string &name) override;
      long moveto(const std::string &name, int axisnum, const std::string &posstr, std::string &retstring) override;
      long get_pos(const std::string &name, int axisnum, float &position, std::string* posname=nullptr, int addr=-1) override;
      /***
      long get_pos(const std::string &name, int axisnum, int addr, float &position, std::string &posname) override;
      long get_pos(const std::string &name, int axisnum, float &position, std::string &posname) override;
      long get_pos(const std::string &name, int axisnum, float &position);
//    long get_pos(const std::string &name, int axisnum, int addr, float &position, std::string &posname);
//    long get_pos(const std::string &name, int axisnum, float &position, std::string &posname);
      long get_pos(const std::string &name, int axisnum, int addr, float &position, std::string &posname, float tol_in);
      ***/

      // PI-only
      //
      long is_home(std::string input, std::string &retstring);
      long is_home(const std::string &name, int addr, int axis);
      long is_home(const std::string &name, int addr, int axis, bool &state);
//    long moveto(std::string motorname, int axisnum, std::string posname, std::string &retstring);
      long moveto(std::string motorname, int axusnum, float position, std::string &retstring);
      long moveto(std::vector<std::string> motornames, std::vector<int> axisnums,
                  std::vector<std::string> posnames, std::string &retstring);
      long clear_errors();
      long get_error(const std::string &name, int addr, int &errcode);
      long set_servo(bool state);
      long set_servo(const std::string &name, bool state);
      long set_servo(const std::string &name, int addr, bool state);
      long set_servo(const std::string &name, int addr, int axis, bool state);
      long on_target(const std::string &name, int addr, bool &retstring);
      long on_target(const std::string &name, int addr, int axis, bool &state);
      long stop_motion(const std::string &name, int addr);
      long send_command(const std::string &motorname, std::string cmd);
      long send_command(const std::string &motorname, std::string cmd, std::string &retstring);

      template <typename ReplyType> long parse_reply( int axis, std::string &reply, ReplyType &retval );
  };
  /***** Interface ************************************************************/

}
/***** Physik_Instrumente *****************************************************/
