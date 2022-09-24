#include "power.h"

namespace Power {

  /**************** Power::NpsInfo::NpsInfo ********8**************************/
  /**
   * @fn         NpsInfo
   * @brief      class constructor
   * @param[in]  none
   * @return     none
   *
   */
  NpsInfo::NpsInfo() {
    this->npsnum=-1;
    this->maxplugs=-1;
    this->host="";
    this->port=-1;
  }
  /**************** Power::NpsInfo::NpsInfo ********8**************************/


  /**************** Power::NpsInfo::~NpsInfo *******8**************************/
  /**
   * @fn         ~NpsInfo
   * @brief      class de-constructor
   * @param[in]  none
   * @return     none
   *
   */
  NpsInfo::~NpsInfo() {
  }
  /**************** Power::NpsInfo::~NpsInfo *******8**************************/


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
   * @param[in]  npsnum
   * @param[in]  cmd
   * @param[out] retstring
   * @return     ERROR or NO_ERROR
   *
   * This function parses the commands and arguments received by the emulator which
   * would need to go to the NPS.
   *
   */
  long Interface::parse_command( int npsnum, std::string cmd, std::string &retstring ) {
    std::string function = "  (Power::Interface::parse_command) ";
    std::stringstream retstream;

    retstream << cmd << "\r\n";

    std::cerr << get_timestamp() << function << "nps" << npsnum << " received command: " << cmd << "\n";

    std::vector<std::string> tokens;
    Tokenize( cmd, tokens, " " );

    std::string mycmd;                 // command is the 1st token
    size_t nargs;                      // number of args (after command)
    int    plugnum;                    // plug number extracted from cmd
    int    set_state=-1;               // requested plug state

    if ( tokens.size() < 1 ) {         // should be impossible since already checked for cmd.empty()
      std::cerr << get_timestamp() << function << "ERROR: nps" << npsnum << " no tokens\n";
      retstream << "NPS>";
      retstring = retstream.str();
      return( ERROR );
    }

    try {
      mycmd = tokens.at(0);            // command is the 1st token
      tokens.erase( tokens.begin() );  // remove the commmand from the vector

      nargs = tokens.size();           // number of args after removing command

      if ( nargs < 1 ) {               // need at least one more
        std::cerr << get_timestamp() << function << "ERROR nps" << npsnum << " missing command arguments\n";
        retstream << "NPS>";
        retstring = retstream.str();
        return( ERROR );
      }

      cmd = tokens.at(0);              // will tokenize this string next
      tokens.clear();                  // erase the tokens
      Tokenize( cmd, tokens, "," );    // get the first arg before the comma -- that should be the plug number

      if ( tokens.size() < 1 ) {
        std::cerr << get_timestamp() << function << "ERROR nps" << npsnum << " missing plug info\n";
        retstream << "NPS>";
        retstring = retstream.str();
        return( ERROR );
      }

      plugnum = std::stoi( tokens.at(0) );  // plug number
    }
    catch( std::invalid_argument &e ) {
      std::cerr << get_timestamp() << function << "ERROR nps" << npsnum 
                                   << " unable to convert one or more values: " << e.what() << "\n";
      retstream << "NPS>";
      retstring = retstream.str();
      return( ERROR );
    }
    catch( std::out_of_range &e ) {
      std::cerr << get_timestamp() << function << "ERROR nps" << npsnum << " one or more values out of range: " << e.what() << "\n";
      retstream << "NPS>";
      retstring = retstream.str();
      return( ERROR );
    }

    /* ------------------------
     * handle the commands here
     * ------------------------
     *
     */

    if ( mycmd == "/ON" )  set_state =  1;
    else
    if ( mycmd == "/OFF" ) set_state =  0;
    else
    if ( mycmd == "/S"   ) set_state = -1;
    else {
      std::cerr << get_timestamp() << function << "ERROR nps" << npsnum << " unrecognized command " << mycmd << "\n";
      retstream << "NPS>";
      retstring = retstream.str();
      return( ERROR );
    }

    // If requested mycmd is ON or OFF
    //
    if ( set_state >= 0 ) {
      this->nps_info.at(npsnum).plugstate.at(plugnum) = set_state;  // save the current state for this plug on this unit
      retstream << "NPS>";
      retstring = retstream.str();
    }

    // otherwise if state is -1 then this is a status request
    //
    else {
      retstream << this->nps_info.at(npsnum).plugstate.at(plugnum) << "\r\nNPS>";
      retstring = retstream.str();
    }

    // return the current state
    //
    std::cerr << get_timestamp() << function << "reply from nps" << npsnum << " emulator: " << retstring << "\n";

#ifdef LOGLEVEL_DEBUG
    for ( auto it = this->nps_info.at(npsnum).plugstate.begin(); it != this->nps_info.at(npsnum).plugstate.end(); ++it ) {
      std::cerr << get_timestamp() << function << "[DEBUG] plugmap for nps" << npsnum << ": "
                << it->first << " " << it->second << "\n";
    }
#endif

    return ( NO_ERROR );
  }
  /**************** Power::Interface::parse_command ***************************/

}
