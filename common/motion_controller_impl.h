/**
 * @file    motion_controller_impl.h
 * @brief   implementations for MotionController::ControllerInfo template class functions
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#pragma once

namespace MotionController {

  /***** ControllerInfo::load_controller_info *********************************/
  /**
   * @brief      Loads information from the config file into the class
   * @details    This is the template class version which can parse the
   *             input string for common parameters, then call the
   *             specialized version which will extract parameters specific
   *             to that type of controller. OBSOLETE
   * @param[in]  input  string containing line from config file
   * @return     ERROR or NO_ERROR
   *
   */
  template <typename T>
  long ControllerInfo<T>::load_controller_info( std::string input ) {
    return T::load_controller_info( input );
  }
  /***** ControllerInfo::load_controller_info *********************************/


  /***** ControllerInfo::load_pos_info ****************************************/
  /**
   * @brief      Loads MOTOR_POS information from config file into the 
   * @details    This is the template class version which will parse the
   *             input string for common parameters.
   * @param[in]  input  string specifies: "<name> <ID> <pos> <posname>"
   * @return     ERROR or NO_ERROR
   *
   * This function is called whenever the MOTOR_POS key is found
   * in the configuration file, to parse and load all of the information
   * assigned by that key into the appropriate class variables.
   *
   * Currently all motors use the same MOTOR_POS format, but if that changes
   * then a template class function call could be made here.
   *
   */
  template <typename T>
  long ControllerInfo<T>::load_pos_info( std::string input ) {
    std::string function = "MotionController::ControllerInfo::load_pos_info";
    int id;
    float position;
    std::string name, posname;

    std::istringstream iss(input);

    if (!(iss >> name
              >> id
              >> position
              >> posname)) {
      logwrite(function, "ERROR bad config input. expected { <name> <ID> <pos> <posname> }");
      return ERROR;
    }

    // ID starts at 0 (can't be negative)
    //
    if ( id < 0 ) {
      std::ostringstream oss;
      oss << "ERROR: id " << id << " for \"" << name << "\" " << "must be >= 0";
      logwrite(function, oss.str());
      return ERROR;
    }

    this->name = name;
    this->posmap[ posname ].id       = id;
    this->posmap[ posname ].position = position;
    this->posmap[ posname ].posname  = posname;

    return NO_ERROR;
  }
  /***** ControllerInfo::load_pos_info ****************************************/

}

