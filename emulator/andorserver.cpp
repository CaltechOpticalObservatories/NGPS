/**
 * @file    andorserver.cpp
 * @brief   these are the main functions for the andor server emulator
 * @author  David Hale <dhale@astro.caltech.edu>
 * @details 
 *
 */

#include "andorserver.h"

namespace AndorServerEmulator {

  /***** AndorServerEmulator::Interface::Interface ****************************/
  /**
   * @fn         Interface
   * @brief      class constructor
   * @param[in]  none
   * @return     none
   *
   */
  Interface::Interface() {
  }
  /***** AndorServerEmulator::Interface::Interface ****************************/


  /***** AndorServerEmulator::Interface::~Interface ***************************/
  /**
   * @fn         ~Interface
   * @brief      class deconstructor
   * @param[in]  none
   * @return     none
   *
   */
  Interface::~Interface() {
  }
  /***** AndorServerEmulator::Interface::~Interface ***************************/


  /***** AndorServerEmulator::Interface::acquire ******************************/
  /**
   * @brief      simulates acquire, which grabs a frame and returns a fits filename
   * @param[in]  cmd
   * @param[out] reply
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::acquire( std::string cmd, std::string &reply ) {
    std::string function = "  (AndorServerEmulator::Interface::acquire) ";
    std::cerr << get_timestamp() << function << "received command: " << cmd << "\n";
    reply = "/tmp/andor.fits";
    return( NO_ERROR );
  }
  /***** AndorServerEmulator::Interface::acquire ******************************/


  /***** AndorServerEmulator::Interface::coords *******************************/
  /**
   * @brief      doesn't have to do anything
   * @param[in]  coordstr
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::coords( std::string coordstr ) {
    std::string function = "  (AndorServerEmulator::Interface::coords) ";
    std::cerr << get_timestamp() << function << "received coordinates: " << coordstr << "\n";
    return( NO_ERROR );
  }
  /***** AndorServerEmulator::Interface::coords *******************************/


}
