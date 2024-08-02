/**
 * @file    power.cpp
 * @brief   these are the main functions for the power emulator
 * @author  David Hale <dhale@astro.caltech.edu>
 * @details 
 *
 */

#include "power.h"

namespace PowerEmulator {

  /***** PowerEmulator::NpsInfo::NpsInfo **************************************/
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
  /***** PowerEmulator::NpsInfo::NpsInfo **************************************/


  /***** PowerEmulator::NpsInfo::~NpsInfo *************************************/
  /**
   * @fn         ~NpsInfo
   * @brief      class de-constructor
   * @param[in]  none
   * @return     none
   *
   */
  NpsInfo::~NpsInfo() {
  }
  /***** PowerEmulator::NpsInfo::~NpsInfo *************************************/


  /***** PowerEmulator::NpsInfo::load_nps_info ********************************/
  /**
   * @fn         load_nps_info
   * @brief      loads NPS information from the configuration file into the class
   * @param[in]  input
   * @return     ERROR or NO_ERROR
   *
   * This function is called whenever the NPS_UNIT key is found in the
   * configuration file, to parse and load all of the information assigned
   * by that key into the appropriate NpsInfo class variables.
   *
   * The input string specifies: "<nps#> <maxplugs> <host> <port>"
   *
   */
  long NpsInfo::load_nps_info( std::string &input, int &npsnum ) {
    std::string function = "PowerEmulator::NpsInfo::load_nps_info";
    std::stringstream message;
    std::vector<std::string> tokens;

    Tokenize( input, tokens, " \"" );

    if ( tokens.size() != 4 ) {
      message.str(""); message << "ERROR bad number of tokens: " << tokens.size() << ". expected 4";
      logwrite( function, message.str() );
      return( ERROR );
    }

    try {
      npsnum         = std::stoi( tokens.at(0) );
      this->maxplugs = std::stoi( tokens.at(1) );
      this->host     = tokens.at(2);
      this->port     = std::stoi( tokens.at(3) );
    }
    catch ( std::invalid_argument &e ) {
      message.str(""); message << "ERROR loading tokens from input: " << input << ": " << e.what();
      logwrite( function, message.str() );
      return( ERROR );
    }
    catch ( std::out_of_range &e ) {
      message.str(""); message << "ERROR loading tokens from input: " << input << ": " << e.what();
      logwrite( function, message.str() );
    }

    if ( npsnum < 0 ) {
      message.str(""); message << "ERROR bad NPS unit number " << npsnum << ": must be >= 0";
      logwrite( function, message.str() );
      return( ERROR );
    }
    else this->npsnum = npsnum;

    return( NO_ERROR );
  }
  /***** PowerEmulator::NpsInfo::load_nps_info ********************************/


  /***** PowerEmulator::NpsInfo::load_plug_info ***************************/
  /**
   * @fn         load_plug_info
   * @brief      loads plug information from the configuration file into the class
   * @param[in]  input
   * @return     ERROR or NO_ERROR
   *
   * This function is called whenever the NPS_PLUG key is found in the
   * configuration file, to parse and load all of the information assigned
   * by that key into the appropriate NpsInfo class variables.
   *
   * The input string specifies: "<nps#> <plug#> <plugname>"
   *
   */
  long NpsInfo::load_plug_info( std::string &input, int &npsnum, int &plugnum, std::string &plugname ) {
    std::string function = "PowerEmulator::NpsInfo::load_plug_info";
    std::stringstream message;
    std::vector<std::string> tokens;

    Tokenize( input, tokens, " \"" );

    if ( tokens.size() != 3 ) {
      message.str(""); message << "ERROR bad number of tokens in \"" << input << "\": expected 3 but received " << tokens.size();
      logwrite( function, message.str() );
      return( ERROR );
    }

    try {
      npsnum      = std::stoi( tokens.at(0) );
      plugnum     = std::stoi( tokens.at(1) );
      plugname    = tokens.at(2);
    }
    catch ( std::invalid_argument &e ) {
      message.str(""); message << "ERROR loading tokens from input: " << input << ": " << e.what();
      logwrite( function, message.str() );
      return( ERROR );
    }
    catch ( std::out_of_range &e ) {
      message.str(""); message << "ERROR loading tokens from input: " << input << ": " << e.what();
      logwrite( function, message.str() );
    }

    if ( npsnum < 0 ) {
      message.str(""); message << "ERROR bad NPS unit number " << npsnum << ": must be >= 0";
      logwrite( function, message.str() );
      return( ERROR );
    }

    if ( plugnum < 0 ) {
      message.str(""); message << "ERROR bad plug number " << plugnum << ": must be >= 0";
      logwrite( function, message.str() );
      return( ERROR );
    }

    if ( plugname.empty() ) {
      logwrite( function, "ERROR plug name cannot be empty" );
      return( ERROR );
    }

    return( NO_ERROR );
  }
  /***** PowerEmulator::NpsInfo::load_plug_info ***************************/

  /***** PowerEmulator::Interface::Interface **********************************/
  /**
   * @fn         Interface
   * @brief      class constructor
   * @param[in]  none
   * @return     none
   *
   */
  Interface::Interface() {
  }
  /***** PowerEmulator::Interface::Interface **********************************/


  /***** PowerEmulator::Interface::~Interface *********************************/
  /**
   * @fn         ~Interface
   * @brief      class deconstructor
   * @param[in]  none
   * @return     none
   *
   */
  Interface::~Interface() {
  }
  /***** PowerEmulator::Interface::~Interface *********************************/


  /***** PowerEmulator::Interface::parse_command ******************************/
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
    std::string function = "  (PowerEmulator::Interface::parse_command) ";
    std::stringstream retstream;

    retstream << cmd << "\r\n";

    std::cerr << get_timestamp() << function << "nps" << npsnum << " received command: " << cmd << "\n";

    std::vector<std::string> tokens;
    Tokenize( cmd, tokens, " " );

    std::string mycmd;                 // command is the 1st token
    size_t nargs;                      // number of args (after command)
    int    plugnum;                    // plug number extracted from cmd for single plug operations
    std::vector<int> plugnums;         // vector of plug number(s) extracted from cmd for multiple plug operations
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

      // plugnum arg may be a range of plug numbers x:y
      // so tokenize on colon to get the first:last plug numbers
      //
      std::vector<std::string> plugnum_str;
      Tokenize( tokens.at(0), plugnum_str, ":" );

      int first_plug = std::stoi( plugnum_str.front() );
      int last_plug  = std::stoi( plugnum_str.back() );

      // Create a vector of plug numbers from first to last
      //
      for ( int plug = first_plug; plug <= last_plug; plug++ ) {
        plugnums.push_back( plug );
      }

      plugnum = first_plug;  // for single plug operations
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

    try {

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
      for ( const auto &plug : plugnums ) {
        retstream << this->nps_info.at(npsnum).plugstate.at( plug ) << ",";       // build return string
      }
      retstring = retstream.str();
      if ( !retstring.empty() && retstring.back() == ',' ) retstring.pop_back();  // remove trailing comma
      retstring += "\r\nNPS>";                                                    // terminate return string
    }

    // return the current state
    //
    std::cerr << get_timestamp() << function << "reply from nps" << npsnum << " emulator: " << retstring << "\n";

#ifdef LOGLEVEL_DEBUG
    for ( auto it = this->nps_info.at(npsnum).plugstate.begin(); it != this->nps_info.at(npsnum).plugstate.end(); ++it ) {
      std::cerr << get_timestamp() << function << "[DEBUG] plugmap for nps" << npsnum << ": "
                << it->first << " " << it->second << "\n";
    }

    }
    catch( std::out_of_range &e ) {
      std::cerr << get_timestamp() << function << "ERROR nps" << npsnum << " plug " << plugnum << ": " << e.what() << "\n";
      retstream << "NPS>";
      retstring = retstream.str();
      return( ERROR );
    }
#endif

    return ( NO_ERROR );
  }
  /***** PowerEmulator::Interface::parse_command ******************************/

}
