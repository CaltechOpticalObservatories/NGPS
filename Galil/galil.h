/**
 * @file    galil.h
 * @brief   this file contains the definitions for the Galil hardware interface
 * @details interfaces to Lakeshore controllers and monitors
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#pragma once

#include "motion_controller.h"
#include "common.h"
#include "network.h"
#include "utilities.h"

#include <string>
#include <vector>
#include <regex>
#include <mutex>

/***** Galil ******************************************************************/
/**
 * @namespace Galil
 * @brief     namespace for the Galil interface
 *
 * This namespace is meant to cover all Galil devices and will contain a class
 * for each specific device type.
 *
 */
namespace Galil {

  // Use common classes directly
  using AxisInfo = MotionController::AxisInfo;  ///< information for a motor axis
  using PosInfo = MotionController::PosInfo;    ///< information for position maps

  template <typename T>
  using ControllerInfo = MotionController::ControllerInfo<T>;

  /***** Galil::SingleAxisInfo ************************************************/
  /**
   * @class    StepperInfo
   * @brief    Derived class containing information specific to single-axis controllers
   *
   */
  class SingleAxisInfo : public MotionController::ControllerInfoBase {
    public:
      SingleAxisInfo() { }
      long load_controller_info();
  };
  /***** Galil::SingleAxisInfo ************************************************/


  /***** Galil::Interface *****************************************************/
  /**
   * @class  Interface
   * @brief  interface class is generic interfacing to Galil hardware via sockets
   *
   */
  template <typename Type>
  class Interface : public MotionController::Interface<Type> {
    public:
      using MotionController::Interface<Type>::Interface;

      void test() override;
      bool is_home(const std::string &name) override;
      long moveto(const std::string &name, int axisnum, const std::string &posstr, std::string &retstring) override;
      long get_pos(const std::string &name, int axisnum, float &position,
                   float tol=NAN, std::string* posname=nullptr, int addr=-1) override;


      long stop(const std::string &name);
      long enable_motion(bool shouldenable);
      long move_to_default();

  };
  /***** Galil::Interface *****************************************************/

}
#include "galil_interface_impl.h"  ///< implementations for Interface template
/***** Galil ******************************************************************/
