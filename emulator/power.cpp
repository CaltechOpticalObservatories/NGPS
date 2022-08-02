#include "power.h"

namespace Power {

  /**************** Power::Interface::Interface *******************************/
  /**
   * @fn         Interface
   * @brief      class constructor
   * @param[in]  none
   * @return     none
   *
   */
  Interface::Interface() {
  }
  /**************** Power::Interface::Interface *******************************/


  /**************** Power::Interface::~Interface ******************************/
  /**
   * @fn         ~Interface
   * @brief      class deconstructor
   * @param[in]  none
   * @return     none
   *
   */
  Interface::~Interface() {
  }
  /**************** Power::Interface::~Interface ******************************/

  /**************** Power::Interface::parse_command ***************************/
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
    std::string function = "(Power::Interface::parse_command) ";

    std::cerr << function << "received command: " << cmd << "\n";

    std::vector<std::string> tokens;
    Tokenize( cmd, tokens, " " );

    std::string mycmd;                 // command is the 1st token
    size_t nargs;                      // number of args (after command)

    if ( tokens.size() < 1 ) {         // should be impossible since already checked for cmd.empty()
      std::cerr << function << "ERROR: no tokens\n";
      retstring = "-2";                // invalid parameters
      return( ERROR );
    }

    try {
      mycmd = tokens.at(0);            // command is the 1st token
      tokens.erase( tokens.begin() );  // remove the commmand from the vector
      nargs = tokens.size();           // number of args after removing command
    }
    catch( std::invalid_argument &e ) {
      std::cerr << function << "unable to convert one or more values: " << e.what() << "\n";
      retstring = "-1";                // unrecognized command
      return( ERROR );
    }
    catch( std::out_of_range &e ) {
      std::cerr << function << "one or more values out of range: " << e.what() << "\n";
      retstring = "-1";                // unrecognized command
      return( ERROR );
    }

    /* ------------------------
     * handle the commands here
     * ------------------------
     *
     */

    // Power to an outlet is turned on/off with the following command:
    // oltctrl index # act on|off
    // where # is the outlet number
    //
    // Anything that fits this format I use to capture the outlet and its state.
    // Anything that doesn't fit this format is ignored.
    //
    if ( mycmd == "oltctrl" ) {
      // four tokens meets the requirements
      //
      if ( nargs == 4 ) {
        int outlet=0;
        int state=-1;
        try {
          if ( tokens.at(1) == "index" ) {
            outlet = std::stoi( tokens.at(2) );
            if ( tokens.at(3) == "on" ) state=1;
            else
            if ( tokens.at(3) == "off" ) state=0;
            else {
              std::cerr << function << "ERROR: state " << tokens.at(3) << " not { on | off }\n";
            }
          }
        }
        catch( std::invalid_argument &e ) {
          std::cerr << function << "unable to convert one or more values: " << e.what() << "\n";
          return( ERROR );
        }
        catch( std::out_of_range &e ) {
          std::cerr << function << "one or more values out of range: " << e.what() << "\n";
          return( ERROR );
        }
//    this->power_mutex.lock();
//    kkkk
//    this->power_mutex.unlock();
/*
      if ( outlet > 0 && outlet < NUM_OUTLETS ) {
      }
      else {
        std::cerr << function << "ERROR: outlet " << outlet << " out of range { 1:" << NUM_OUTLETS << " }\n";
      }
*/
      }
      else {
        std::cerr << function << "ignoring command " << cmd << "\n";
      }
    }
    else {
      std::cerr << function << "ignoring command " << cmd << "\n";
    }

    std::cerr << function << "reply from power emulator: " << retstring << "\n";

    return ( NO_ERROR );
  }
  /**************** Power::Interface::parse_command ***************************/

}
