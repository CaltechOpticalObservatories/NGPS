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

      long send_command(const std::string &motorname, std::string cmd, std::string* retstring=nullptr);

  };
  /***** Galil::Interface *****************************************************/

}
/***** Galil ******************************************************************/
