/**
 * @file    telem_interface.cpp
 * @brief   this contains the telemetry interface code
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 * This file contains the code for the Interface class in the Telemetry namespace.
 *
 */

#include "telem_interface.h"

namespace Telemetry {


  /***** Telemetry::Interface::Interface *******************************************/
  /**
   * @brief      class constructor
   * @details    The andor camera will be initialized upon construction
   */
  Interface::Interface() {
  }
  /***** Telemetry::Interface::Interface *******************************************/


  /***** Telemetry::Interface::~Interface ******************************************/
  /**
   * @brief      class deconstructor
   * @details    The andor camera will be closed upon de-construction
   *
   */
  Interface::~Interface() {
  }
  /***** Telemetry::Interface::~Interface ******************************************/


  /***** Telemetry::Interface::configure_interface *********************************/
  /**
   * @brief      configure the interface from the .cfg file
   * @details    this function can be called at any time, e.g. from HUP or a command
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::configure_interface( Config &config ) {
    std::string function = "Telemetry::Interface::configure_interface";
    std::stringstream message;
    int applied=0;
    long error = NO_ERROR;

    // loop through the entries in the configuration file, stored in config class
    //
    for (int entry=0; entry < config.n_entries; entry++) {
    }
    message << "applied " << applied << " configuration lines to the telemetry interface";
    logwrite(function, message.str());
    return error;
  }
  /***** Telemetry::Interface::configure_interface *********************************/


}
