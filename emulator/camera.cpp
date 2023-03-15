#include "camera.h"

namespace Camera {

  /**************** Camera::Interface::Interface ******************************/
  /**
   * @fn         Interface
   * @brief      class constructor
   * @param[in]  none
   * @return     none
   *
   */
  Interface::Interface() {
  }
  /**************** Camera::Interface::Interface ******************************/


  /**************** Camera::Interface::~Interface *****************************/
  /**
   * @fn         ~Interface
   * @brief      class deconstructor
   * @param[in]  none
   * @return     none
   *
   */
  Interface::~Interface() {
  }
  /**************** Camera::Interface::~Interface *****************************/


  /**************** Camera::Interface::parse_command **************************/
  /**
   * @fn         parse_command
   * @brief      parse commands for the NPS, spawn a thread if necessary
   * @param[in]  cmd
   * @param[out] retstring
   * @return     ERROR or NO_ERROR
   *
   * This function parses the commands and arguments received by the emulator which
   * would need to go to the NPS.
   *
   */
  long Interface::parse_command( std::string cmd, std::string &retstring ) {
    std::string function = "  (Camera::Interface::parse_command) ";

    std::cerr << get_timestamp() << function << "received command: " << cmd << "\n";

    std::vector<std::string> tokens;
    Tokenize( cmd, tokens, " " );

    std::string mycmd;                 // command is the 1st token
    size_t nargs;                      // number of args (after command)

    if ( tokens.size() < 1 ) {         // should be impossible since already checked for cmd.empty()
      std::cerr << get_timestamp() << function << "ERROR: no tokens\n";
      retstring = "-2";                // invalid parameters
      return( ERROR );
    }

    try {
      mycmd = tokens.at(0);            // command is the 1st token
      tokens.erase( tokens.begin() );  // remove the commmand from the vector
      nargs = tokens.size();           // number of args after removing command
    }
    catch( std::invalid_argument &e ) {
      std::cerr << get_timestamp() << function << "unable to convert one or more values: " << e.what() << "\n";
      retstring = "-1";                // unrecognized command
      return( ERROR );
    }
    catch( std::out_of_range &e ) {
      std::cerr << get_timestamp() << function << "one or more values out of range: " << e.what() << "\n";
      retstring = "-1";                // unrecognized command
      return( ERROR );
    }

    /* ------------------------
     * handle the commands here
     * ------------------------
     *
     */

    if ( mycmd == "" ) {
    }
    else {
      std::cerr << get_timestamp() << function << "ignoring command " << cmd << "\n";
    }

    std::cerr << get_timestamp() << function << "reply from camera emulator: " << retstring << "\n";

    return ( NO_ERROR );
  }
  /**************** Camera::Interface::parse_command **************************/

}
