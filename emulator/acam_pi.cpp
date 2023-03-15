/**
 * @file    acam_pi.cpp
 * @brief   these are the functions for the Acam PI emulator
 * @details 
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#include "acam_pi.h"

namespace AcamPIEmulator {

  /***** AcamPIEmulator::Interface::Interface *********************************/
  /**
   * @fn         Interface
   * @brief      class constructor
   * @param[in]  none
   * @return     none
   *
   */
  Interface::Interface() {
  }
  /***** AcamPIEmulator::Interface::Interface *********************************/


  /***** AcamPIEmulator::Interface::~Interface ********************************/
  /**
   * @fn         ~Interface
   * @brief      class deconstructor
   * @param[in]  none
   * @return     none
   *
   */
  Interface::~Interface() {
  }
  /***** AcamPIEmulator::Interface::~Interface ********************************/


  long Interface::test() {
    std::string function = "  (AcamPIEmulator::Interface::test) ";
    std::cerr << get_timestamp() << function << "\n";
    return( NO_ERROR );
  }


  /***** AcamPIEmulator::Interface::parse_command *****************************/
  /**
   * @brief      parse the incomming command and arguments
   * @param[in]  cmd
   * @param[out] retstring
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::parse_command( std::string cmd, std::string &retstring ) {
    std::string function = "  (AcamPIEmulator::Interface::parse_command) ";

    std::cerr << get_timestamp() << function << "received command: " << cmd << "\n";

    std::vector<std::string> tokens;
    Tokenize( cmd, tokens, " " );

    try {
    }
    catch( std::invalid_argument &e ) {
      std::cerr << get_timestamp() << function << "unable to convert one or more values: " << e.what() << "\n";
      return( ERROR );
    }
    catch( std::out_of_range &e ) {
      std::cerr << get_timestamp() << function << "one or more values out of range: " << e.what() << "\n";
      return( ERROR );
    }

    /* ------------------------
     * handle the commands here
     * ------------------------
     *
     */


    return ( NO_ERROR );
  }
  /***** AcamPIEmulator::Interface::parse_command *****************************/

}
