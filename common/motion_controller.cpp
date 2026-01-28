/**
 * @file    motion_controller.cpp
 * @brief   this file contains the code for the motion controller interface
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#include "motion_controller.h"
#include "logentry.h"

namespace MotionController {

  /***** MotionController::AxisInfo::load_axis_info ***************************/
  /**
   * @brief      send a command string to the controller
   * @details    parses the config file MOTOR_AXIS key and loads into class
   * @param[in]  input  single row from config file for key "MOTOR_AXIS"
   * @return     ERROR|NO_ERROR
   *
   */
  long AxisInfo::load_axis_info(const std::string &input) {
    std::string function = "MotionController::AxisInfo::load_axis_info";

    std::string tryname, tryreftype;
    float trymin, trymax, tryzero, trydefpos;
    int trynum;

    std::istringstream iss(input);

    // get the required parameters
    //
    if ( !(iss >> tryname
               >> trynum
               >> trymin
               >> trymax
               >> tryzero
               >> tryreftype) ) {
      logwrite(function, "ERROR: bad config input. Expected { <motorname> <axis> <min> <max> <zero> <ref> [ <defpos> ]}");
      return ERROR;
    }

    // the default position is optional
    //
    if ( !(iss >> trydefpos) ) {
      trydefpos=NAN;
    }

    // The requested reftype must be one of the PI valid reftypes.
    // Use "na" if the axis doesn't support referencing.
    //
    const std::vector<std::string> valid_reftypes{ "neg", "pos", "ref", "na" };

    if ( std::find( valid_reftypes.begin(), valid_reftypes.end(), tryreftype ) == valid_reftypes.end() ) {
      std::ostringstream oss;
      oss << "ERROR: reftype \"" << tryreftype << "\" invalid. Expected { ";
      for ( auto ref : valid_reftypes ) oss << ref << " ";
      oss << "}";
      logwrite(function, oss.str());
      return ERROR;
    }

    // num must be 1-based
    //
    if ( trynum < 1 ) {
      logwrite(function, "ERROR axis number must be > 0: \""+input+"\"");
      return ERROR;
    }

    // min can't be greater than max
    //
    if ( trymin > trymax ) {
      logwrite(function, "ERROR min can't be greater than max \""+input+"\"");
      return ERROR;
    }

    // all valid so set the class variables
    //
    this->motorname = tryname;
    this->axisnum   = trynum;
    this->reftype   = tryreftype;
    this->min       = trymin;
    this->max       = trymax;
    this->zeropos   = tryzero;
    this->defpos    = trydefpos;

    return NO_ERROR;
  }
  /***** MotionController::AxisInfo::load_axis_info ***************************/


  /***** MotionController::PosInfo::load_pos_info *****************************/
  /**
   * @brief      Loads MOTOR_POS information from config file into the 
   * @details    parses the config file MOTOR_POS key and loads into the class
   * @param[in]  input  string specifies: "<motorname> <ID> <pos> <posname>"
   * @return     ERROR or NO_ERROR
   *
   * This function is called whenever the MOTOR_POS key is found
   * in the configuration file, to parse and load all of the information
   * assigned by that key into the appropriate class variables.
   *
   */
  long PosInfo::load_pos_info(const std::string &input) {
    std::string function = "MotionController::PosInfo::load_pos_info";

    int axis, posid;
    float position;
    std::string motorname, posname;

    std::istringstream iss(input);

    if (!(iss >> motorname
              >> axis
              >> posid
              >> position
              >> posname)) {
      logwrite(function, "ERROR bad config input. expected { <motorname> <axis> <posid> <pos> <posname> }");
      return ERROR;
    }

    // ID starts at 0 (can't be negative)
    //
    if ( posid < 0 ) {
      std::ostringstream oss;
      oss << "ERROR: posid " << posid << " for \"" << this->motorname << "\" "
          << "must be >= 0";
      logwrite(function, oss.str());
      return ERROR;
    }

    this->motorname = motorname;
    this->axis      = axis;
    this->posid     = posid;
    this->position  = position;
    this->posname   = posname;

    return NO_ERROR;
  }
  /***** MotionController::PosInfo::load_pos_info *****************************/

}
