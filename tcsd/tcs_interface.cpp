/**
 * @file    tcs_interface.cpp
 * @brief   this contains the tcs interface code
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 * This file contains the code for the Interface class in the TCS namespace,
 * and is how the tcs daemon interfaces to the actual TCS hardware.
 *
 */

#include "tcs_interface.h"

namespace TCS {

  /***** TCS::Interface::list *************************************************/
  /**
   * @brief      list configured TCS devices
   * @param[in]  arg        arg used only for requesting help
   * @param[out] retstring  reference to string to contain the list of devices
   *
   */
  void Interface::list( const std::string &arg, std::string &retstring ) {
    std::string function = "TCS::Interface::list";
    std::stringstream message;

    // Help
    //
    if ( arg == "?" || arg == "help" ) {
      retstring = TCSD_LIST;
      retstring.append( " \n" );
      retstring.append( "  List configured TCS devices.\n" );
      retstring.append( "  Format will be <name> <host>:<port> <connected>, one per line.\n" );
      return;
    }

    message << "name host:port connected\n";

    for ( auto const &[key,val] : this->tcsmap ) {
      message << val.name() << " " 
              << val.host() << ":" 
              << val.port() << " "
              << ( val.isconnected() ? "true" : "false" ) << "\n";
    }

    retstring = message.str();

    return;
  }
  /***** TCS::Interface::list *************************************************/


  /***** TCS::Interface::llist ************************************************/
  /**
   * @brief      line list configured TCS devices
   * @details    same as list except all results come out on a single line
   * @param[in]  arg        arg used only for requesting help
   * @param[out] retstring  reference to string to contain the list of devices
   *
   */
  void Interface::llist( const std::string &arg, std::string &retstring ) {
    std::string function = "TCS::Interface::llist";
    std::stringstream message, asyncmsg;

    // Help
    //
    if ( arg == "?" || arg == "help" ) {
      retstring = TCSD_LLIST;
      retstring.append( " \n" );
      retstring.append( "  List configured TCS devices.\n" );
      retstring.append( "  Format will be <name> <host>:<port> <connected>, for each device,\n" );
      retstring.append( "  separated by a comma (all on a single line, including the DONE|ERROR).\n" );
      return;
    }

    int count=0;
    for ( auto const &[key,val] : this->tcsmap ) {
      if ( count++ > 0 ) message << ",";  // add a comma separator if coming through here again
      message << val.name() << " " 
              << val.host() << ":" 
              << val.port() << " "
              << ( val.isconnected() ? "true" : "false" );
    }

    retstring = message.str();

    asyncmsg << "TCSD:llist:" << retstring;
    this->async.enqueue( asyncmsg.str() );

    return;
  }
  /***** TCS::Interface::llist ************************************************/


  /***** TCS::Interface::open *************************************************/
  /**
   * @brief      opens the TCS socket connection
   * @param[in]  arg  contains what to open, real or sim
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::open( const std::string &arg, std::string &retstring ) {
    std::string function = "TCS::Interface::open";
    std::stringstream message, asyncmsg;
    long error = NO_ERROR;

    // Help
    //
    if ( arg == "?" ) {
      retstring = TCSD_OPEN;
      retstring.append( " <name>\n" );
      retstring.append( "  Open a connection to the specified TCS device, where\n" );
      retstring.append( "  <name> = { " );
      message.str("");
      for ( auto const &[key,val] : this->tcsmap ) message << val.name() << " ";
      message << "}\n";
      retstring.append( message.str() );
      return( NO_ERROR );
    }

    // Typically, a second call to open might not return an error but this function returns an error
    // on a second call to open because the user might be trying to open a different TCS, e.g.
    // opening the real while the sim is already open, so returning an error will catch that.
    //
    for ( auto const &[key,val] : this->tcsmap ) {
      if ( val.isconnected() ) {
        message.str(""); message << "ERROR: connection already open to " << val.name() << " "
                                 << val.host() << ":" << val.port();
        logwrite( function, message.str() );
        retstring="already_open";
        return( ERROR );
      }
    }

    // Need the name of the tcs to connect to
    //
    if ( arg.empty() ) {
      message.str(""); message << "ERROR: must specify a valid TCS name";
      logwrite( function, message.str() );
      retstring="missing_argument";
      return( ERROR );
    }

    // Find the requested name in the tcsmap
    //
    auto tcsloc = this->tcsmap.find( arg );

    if ( tcsloc == this->tcsmap.end() ) {
      message.str(""); message << "ERROR: requested TCS name \"" << arg << "\" not found in configured list";
      logwrite( function, message.str() );
      retstring="invalid_argument";
      return( ERROR );
    }

    // initialize connection to the TCS
    //
    message.str(""); message << "opening connection to TCS " << tcsloc->first << " on "
                             << tcsloc->second.host() << ":" << tcsloc->second.port();
    logwrite( function, message.str() );

    if ( tcsloc->second.open() != 0 ) {
      message.str(""); message << "ERROR connecting to TCS " << tcsloc->first << " on "
                               << tcsloc->second.host() << ":" << tcsloc->second.port();
      logwrite( function, message.str() );
      return( ERROR );
    }

    message.str(""); message << "connected to " << tcsloc->second.name() << " "
                             << tcsloc->second.host() << ":" << tcsloc->second.port()
                             << " on fd " << tcsloc->second.fd();
    logwrite( function, message.str() );

    this->name = tcsloc->second.name();  // save the name of the opened tcs

    error = this->isopen( retstring );

    asyncmsg << "TCSD:open:" << ( !retstring.empty() ? retstring : "ERROR" );
    this->async.enqueue( asyncmsg.str() );

    return( error );
  }
  /***** TCS::Interface::open *************************************************/


  /***** TCS::Interface::isopen ***********************************************/
  /**
   * @brief      return the open status of the TCS
   * @return     true | false
   *
   */
  bool Interface::isopen() {
    std::string ret;
    this->isopen( ret );
    return( ret == "false" ? false : true );
  }
  /***** TCS::Interface::isopen ***********************************************/


  /***** TCS::Interface::isopen ***********************************************/
  /**
   * @brief      return the open status of the TCS
   * @param[out] retstring  reference to string contains open status
   * @return     ERROR or NO_ERROR
   *
   * status string is either "false" or the name of the TCS if open.
   *
   */
  long Interface::isopen( std::string &retstring ) {
    return this->isopen( "", retstring );
  }
  /***** TCS::Interface::isopen ***********************************************/


  /***** TCS::Interface::isopen ***********************************************/
  /**
   * @brief      return the open status of the TCS
   * @param[in]  arg        input arg used for help
   * @param[out] retstring  reference to string contains open status
   * @return     ERROR or NO_ERROR
   *
   * status string is either "false" or the name of the TCS if open.
   *
   */
  long Interface::isopen( const std::string &arg, std::string &retstring ) {
    std::string function = "TCS::Interface::isopen";
    std::stringstream message, asyncmsg;

    // Help
    //
    if ( arg == "?" || arg == "help" ) {
      retstring = TCSD_ISOPEN;
      retstring.append( " \n" );
      retstring.append( "  Returns the open status of the TCS. Response will\n" );
      retstring.append( "  either be \"false\" if closed, or the name of the TCS\n" );
      retstring.append( "  if open { real | sim }.\n" );
      return( NO_ERROR );
    }

    for ( auto const &[key,val] : this->tcsmap ) {
      if ( val.isconnected() ) {
#ifdef LOGLEVEL_DEBUG
        message.str(""); message << "[DEBUG] connection open to " << val.name() << " on "
                                 << val.host() << ":" << val.port();
        logwrite( function, message.str() );
#endif
        retstring = val.name();
        return( NO_ERROR );
      }
    }

    retstring = "false";
    return( NO_ERROR );
  }
  /***** TCS::Interface::isopen ***********************************************/


  /***** TCS::Interface::close ************************************************/
  /**
   * @brief      closes the TCS socket connection
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::close( ) {
    std::string function = "TCS::Interface::close";
    std::stringstream message, asyncmsg;
    long error = NO_ERROR;

    for ( auto const &[key,val] : this->tcsmap ) {
      if ( val.isconnected() ) {
        message.str(""); message << "closing connection to " << val.name() << " "
                                 << val.host() << ":" << val.port();
        logwrite( function, message.str() );
        error = val.close();
        this->name.clear();  // clear the name of the opened tcs
      }
    }

    return( error );
  }
  /***** TCS::Interface::close ************************************************/


  /***** TCS::Interface::get_weather_coords ***********************************/
  /**
   * @brief      get the current simulator coords
   * @details    uses the ?WEATHER command, pulls out just the RA and DEC
   * @param[in]  arg        used only to request help
   * @param[out] retstring  contains space-delimited ra dec
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::get_weather_coords( const std::string &arg, std::string &retstring ) {
    std::string function = "TCS::Interface::get_weather_coords";
    std::stringstream message, asyncmsg;
    std::string weather;
    long error = NO_ERROR;

    // Help
    //
    if ( arg == "?" ) {
      retstring = TCSD_WEATHER_COORDS;
      retstring.append( " \n" );
      retstring.append( "  Return the current decimal RA DEC as h.hhhhhhhh d.dddddddd\n" );
      return( NO_ERROR );
    }

    // Send the WEATHER command to the TCS. This returns a string of key=val pairs
    // with each pair separated by a newline character. The first two pairs are 
    // RA and DEC, which is all that is needed here.
    //
    if ( error != ERROR && this->send_command( "?WEATHER", weather ) != NO_ERROR ) {
      message << "ERROR getting coords from TCS";
      error = ERROR;
    }

    std::vector<std::string> pairs;
    Tokenize( weather, pairs, "\n" );  // tokenize on the newline to break into key=val pairs

    // If someone ever changes the TCS message then this will have to be changed.
    //
    if ( error != ERROR && pairs.size() != 14 ) {
      message << "ERROR malformed reply from TCS";
      error = ERROR;
    }

    // Tokenize the first two key=val pairs on the "=" to get the ra, dec values
    //
    if ( error != ERROR ) {
    try {
      std::vector<std::string> ra_tokens, dec_tokens;

      Tokenize( pairs.at(0), ra_tokens, "=" );
      Tokenize( pairs.at(1), dec_tokens, "=" );

      message << ra_tokens.at(1) << " " << dec_tokens.at(1);
    }
    catch( std::out_of_range &e ) {
      logwrite( function, "ERROR out of range parsing ra,dec from ?WEATHER command" );
      return ERROR;
    }
    }

    logwrite( function, message.str() );

    retstring = message.str();

    asyncmsg << "TCSD:weathercoords:" << ( error==NO_ERROR ? message.str() : "ERROR" );
    this->async.enqueue( asyncmsg.str() );

    return error;
  }
  /***** TCS::Interface::get_weather_coords ***********************************/


  /***** TCS::Interface::get_coords *******************************************/
  /**
   * @brief      get the current coords
   * @details    uses the REQPOS command, pulls out just the RA and DEC,
   *             returns them as "hh:mm:ss.ss dd:mm:ss.ss"
   * @param[in]  arg        used only to request help
   * @param[out] retstring  contains space-delimited ra dec
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::get_coords( const std::string &arg, std::string &retstring ) {
    std::string function = "TCS::Interface::get_coords";
    std::stringstream message, asyncmsg;
    long error = NO_ERROR;

    // Help
    //
    if ( arg == "?" ) {
      retstring = TCSD_GET_COORDS;
      retstring.append( " \n" );
      retstring.append( "  Return the current RA DEC as hh:mm:ss.ss dd:mm:ss.ss\n" );
      return( NO_ERROR );
    }

    // Send the REQPOS command to the TCS. This returns a string that looks like:
    //
    // tcsreply = "UTC = ddd hh:mm:ss, LST = hh:mm:ss\n
    //             RA = hh:mm:ss.ss, DEC = dd:mm:ss.s, HA=hh:mm:ss.s\n
    //             air mass = aa.aaa"
    //
    std::string tcsreply;
    if ( this->send_command( "REQPOS", tcsreply ) != NO_ERROR ) {
      logwrite( function, "ERROR getting coords from TCS" );
      error = ERROR;
    }

    // First tokenize on newline
    //
    std::vector<std::string> lines;
    Tokenize( tcsreply, lines, "\n" );

    if ( error != ERROR && lines.size() != 3 ) {
      message.str(""); message << "ERROR expected 3 lines from REQPOS string but received " << lines.size();
      logwrite( function, message.str() );
      error = ERROR;
    }

    // Then the 2nd token, lines[1], contains the RA,DEC so try to
    // tokenize that on the comma.
    //
    try {
      std::vector<std::string> radec;
      Tokenize( lines.at(1), radec, "," );  // lines[1] = "RA = hh:mm:ss.ss, DEC = dd:mm:ss.s, HA=hh:mm:ss.s\n"

      if ( error != ERROR && radec.size() != 3 ) {
        message.str(""); message << "ERROR expected 3 tokens for ra, dec, ha but received " << radec.size();
        logwrite( function, message.str() );
        error = ERROR;
      }

      // Now try to tokenize the first two tokens of line1 on the equal sign to extract RA and DEC values
      //
      if ( error != ERROR ) {
        std::vector<std::string> ra, dec;
        Tokenize( radec.at(0), ra,  "=" );
        Tokenize( radec.at(1), dec, "=" );

        if ( ra.size() != 2 && dec.size() != 2 ) {
          message.str(""); message << "ERROR extracting ra,dec from " << lines.at(1);
          logwrite( function, message.str() );
          error = ERROR;
        }
        message.str(""); message << ra.at(1) << " " << dec.at(1);
        retstring = message.str();
      }
    }
    catch( std::out_of_range &e ) {
      logwrite( function, "EXCEPTION: out of range parsing ra,dec from TCS reply string" );
      error = ERROR;
    }

    if ( !retstring.empty() ) logwrite( function, retstring );

    asyncmsg << "TCSD:getcoords:" << ( !retstring.empty() ? retstring : "ERROR" );
    this->async.enqueue( asyncmsg.str() );

    return error;
  }
  /***** TCS::Interface::get_coords *******************************************/


  /***** TCS::Interface::get_cass *********************************************/
  /**
   * @brief      get the current cass angle
   * @details    uses the REQSTAT command, pulls out just the Cass ring angle,
   *             returns as "ddd.dd"
   * @param[in]  arg        used only to request help
   * @param[out] retstring  contains cass ring angle
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::get_cass( const std::string &arg, std::string &retstring ) {
    std::string function = "TCS::Interface::get_cass";
    std::stringstream message, asyncmsg;
    std::stringstream reply;
    long error = NO_ERROR;

    // Help
    //
    if ( arg == "?" ) {
      retstring = TCSD_GET_CASS;
      retstring.append( " \n" );
      retstring.append( "  Return the current cass ring angle\n" );
      return( NO_ERROR );
    }

    // Send the REQSTAT command to the TCS. This returns a string that looks like:
    //
    // tcsreply = "UTC = 127 21:58:7.0\n
    //             telescope ID = 200, focus = 36.71 mm, tube length = 22.11 mm\n
    //             offset RA = 45.00 arcsec, DEC = 45.00 arcsec\n
    //             rate RA = 0.00 arcsec/hr, DEC = 0.00 arcsec/hr\n
    //             Cass ring angle = 18.00
    //
    std::string tcsreply;
    if ( this->send_command( "REQSTAT", tcsreply ) != NO_ERROR ) {
      logwrite( function, "ERROR getting status from TCS" );
      error = ERROR;
    }

    // First tokenize on comma
    //
    std::vector<std::string> lines;
    Tokenize( tcsreply, lines, "\n" );

    if ( error==NO_ERROR && lines.size() != 5 ) {
      message.str(""); message << "ERROR expected 5 lines from REQSTAT string but received " << lines.size()
                               << " in reply \"" << tcsreply << "\"";
      logwrite( function, message.str() );
      error = ERROR;
    }

    // Then the 5th token, lines[4], contains the "Cass ring angle = dd.dd"
    // tokenize that on the equal to get the value.
    //
    std::string cassline;                    // assign this a variable for exception reporting
    try {
      cassline = lines.at(4);                // lines[4] = "Cass ring angle = dd.dd"
      std::vector<std::string> tokens;
      Tokenize( cassline, tokens, "=" );

      if ( error==NO_ERROR && tokens.size() != 2 ) {
        message.str(""); message << "ERROR expected 2 tokens for Cass ring angle = dd.dd but received " << tokens.size();
        logwrite( function, message.str() );
        error = ERROR;
      }

      // Try converting it to a double, not because it's needed as a double but this checks
      // that it is indeed a double
      //
      if ( error==NO_ERROR ) {
        double angle = std::stod( tokens.at(1) );
        reply << std::fixed << std::setprecision(2) << angle;
      }
    }
    catch( std::out_of_range &e ) {
      message.str(""); message << "EXCEPTION: out of range parsing Cass ring angle from TCS reply string \"" << cassline << "\"";
      logwrite( function, message.str() );
      error = ERROR;
    }
    catch( std::invalid_argument &e ) {
      message.str(""); message << "EXCEPTION converting Cass ring angle from \"" << cassline << "\" to double";
      logwrite( function, message.str() );
      error = ERROR;
    }

    retstring = reply.str();

    if ( !retstring.empty() ) logwrite( function, retstring );

    asyncmsg << "TCSD:getcass:" << ( !retstring.empty() ? retstring : "ERROR" );
    this->async.enqueue( asyncmsg.str() );

    return error;
  }
  /***** TCS::Interface::get_cass *********************************************/


  /***** TCS::Interface::get_dome *********************************************/
  /**
   * @brief      get the dome and telescope azimuths
   * @details    uses the ?WEATHER command, pulls out just the dome and tel azimuth
   *             returns them as "<domeazi> <telazi>"
   * @param[in]  arg        used only to request help
   * @param[out] retstring  contains space-delimited ra dec
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::get_dome( const std::string &arg, std::string &retstring ) {
    std::string function = "TCS::Interface::get_dome";
    std::stringstream message, asyncmsg;
    std::string weather;
    long error = NO_ERROR;

    // Help
    //
    if ( arg == "?" ) {
      retstring = TCSD_GET_DOME;
      retstring.append( " \n" );
      retstring.append( "  Return the dome and telescope azimuth positions.\n" );
      return( NO_ERROR );
    }

    // Send the WEATHER command to the TCS. This returns a string of key=val pairs
    // with each pair separated by a newline character. The first two pairs are 
    // RA and DEC, which is all that is needed here.
    //
    if ( error != ERROR && this->send_command( "?WEATHER", weather ) != NO_ERROR ) {
      logwrite( function, "ERROR getting coords from TCS" );
      error = ERROR;
    }

    std::vector<std::string> pairs;
    Tokenize( weather, pairs, "\n" );  // tokenize on the newline to break into key=val pairs

    // If someone ever changes the TCS message then this will have to be changed.
    //
    if ( error != ERROR && pairs.size() != 14 ) {
      logwrite( function, "ERROR malformed reply" );
      error = ERROR;
    }

    // Tokenize the first two key=val pairs on the "=" to get the ra, dec values
    //
    try {
      if ( error != ERROR ) {
        std::vector<std::string> telazi_tokens, domeazi_tokens;

        Tokenize( pairs.at(5), telazi_tokens, "=" );
        Tokenize( pairs.at(8), domeazi_tokens, "=" );

        message.str(""); message << domeazi_tokens.at(1) << " " << telazi_tokens.at(1);
        retstring = message.str();
      }
    }
    catch( std::out_of_range &e ) {
      logwrite( function, "ERROR out of range parsing azimuth from ?WEATHER command" );
      error = ERROR;
    }

    if ( !retstring.empty() ) logwrite( function, retstring );

    asyncmsg << "TCSD:getdome:" << ( !retstring.empty() ? retstring : "ERROR" );
    this->async.enqueue( asyncmsg.str() );

    return error;
  }
  /***** TCS::Interface::get_dome *********************************************/


  /***** TCS::Interface::set_focus ********************************************/
  /**
   * @brief      set the focus position
   * @details    uses the FOCUSGO command
   * @param[in]  arg        requested focus value
   * @param[out] retstring  reference to return string for the command sent to the TCS
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::set_focus( const std::string &arg, std::string &retstring ) {
    std::string function = "TCS::Interface::set_focus";
    std::stringstream message, asyncmsg;
    long error = NO_ERROR;

    // Help
    //
    if ( arg == "?" || arg == "help" ) {
      retstring = TCSD_SET_FOCUS;
      retstring.append( " <value>\n" );
      retstring.append( "  Set the telescope focus position in mm within range {1:74}\n" );
      return( NO_ERROR );
    }

    if ( arg.empty() ) { retstring="missing_argument"; return ERROR; }

    double value;

    try {
      value = std::stod( arg );
    }
    catch( std::out_of_range &e ) {
      message.str(""); message << "ERROR parsing focus value from \"" << arg << "\": " << e.what();
      logwrite( function, message.str() );
      error = ERROR;
    }
    catch( std::invalid_argument &e ) {
      message.str(""); message << "ERROR parsing focus value from \"" << arg << "\": " << e.what();
      logwrite( function, message.str() );
      error = ERROR;
    }

    if ( std::isnan(value) ) {
      logwrite( function, "ERROR: requested focus is NaN" );
      retstring="bad_value";
      return( ERROR );
    }

    if ( value < 1.0 || value > 74.0 ) {
      message.str(""); message << "ERROR requested focus " << value << " outside range { 1:74 }";
      logwrite( function, message.str() );
      retstring="outside_range";
      return( ERROR );
    }

    std::stringstream cmd;
    std::string retcode;
    cmd << "FOCUSGO " << std::fixed << std::setprecision(2) << value;

    if ( error != ERROR ) error = this->send_command( cmd.str(), retcode );

    this->parse_reply_code( retcode, retstring );

    asyncmsg << "TCSD:focusgo:" << retstring;
    this->async.enqueue( asyncmsg.str() );

    return error;
  }
  /***** TCS::Interface::set_focus ********************************************/


  /***** TCS::Interface::get_focus ********************************************/
  /**
   * @brief      get the current focus position
   * @details    uses this version internally, when help won't be requested
   * @param[out] retstring  contains focus position
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::get_focus( std::string &retstring ) {
    std::string notneeded;
    return get_focus( notneeded, retstring );
  }
  /***** TCS::Interface::get_focus ********************************************/


  /***** TCS::Interface::get_focus ********************************************/
  /**
   * @brief      get the current focus position
   * @details    uses the REQSTAT command, pulls out just the focus,
   *             returns as "dd.dd". Set arg="poll" to suppress logging.
   * @param[in]  arg        input arg { help | poll }
   * @param[out] retstring  contains focus position.
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::get_focus( const std::string &arg, std::string &retstring ) {
    std::string function = "TCS::Interface::get_focus";
    std::stringstream message, asyncmsg;
    std::stringstream reply;
    long error = NO_ERROR;

    // Help
    //
    if ( arg == "?" || arg == "help" ) {
      retstring = TCSD_GET_FOCUS;
      retstring.append( " \n" );
      retstring.append( "  Return the current telescope focus position in mm\n" );
      return( NO_ERROR );
    }

    bool silent = false;                 // logging enabled by default

    if ( arg == "poll" ) silent = true;  // logging is suppressed for polling

    // Send the REQSTAT command to the TCS. This returns a string that looks like:
    //
    // tcsreply = "UTC = 127 21:58:7.0\n
    //             telescope ID = 200, focus = 36.71 mm, tube length = 22.11 mm\n
    //             offset RA = 45.00 arcsec, DEC = 45.00 arcsec\n
    //             rate RA = 0.00 arcsec/hr, DEC = 0.00 arcsec/hr\n
    //             Cass ring angle = 18.00
    //
    std::string tcsreply;
    if ( this->send_command( "REQSTAT", tcsreply ) != NO_ERROR ) {
      logwrite( function, "ERROR getting status from TCS" );
      error = ERROR;
    }

    // First tokenize on comma
    //
    std::vector<std::string> lines;
    Tokenize( tcsreply, lines, "\n" );

    if ( error==NO_ERROR && lines.size() != 5 ) {
      message.str(""); message << "ERROR expected 5 lines from REQSTAT string but received " << lines.size()
                               << " in reply \"" << tcsreply << "\"";
      logwrite( function, message.str() );
      error = ERROR;
    }

    // Then the 2nd token, lines[1], contains the "telescope ID = 200, focus = 36.71 mm, tube length = 22.11 mm\n"
    // tokenize that on the comma to get the params from that line.
    //
    try {
      std::vector<std::string> params, tokens;
      Tokenize( lines.at(1), params, "," );  // lines[1] = "telescope ID = 200, focus = 36.71 mm, tube length = 22.11 mm\n"

      if ( error==NO_ERROR && params.size() != 3 ) {
        message.str(""); message << "ERROR expected 3 params on line 2 but received " << params.size();
        logwrite( function, message.str() );
        error = ERROR;
      }
      else Tokenize( params.at(1), tokens, "=" );  // Finally, tokenize on "=" to get the value from params[1] = "focus = 36.71 mm"

      if ( error==NO_ERROR && tokens.size() != 2 ) {
        message.str(""); message << "ERROR expected 2 tokens for focus = dd.dd mm but received " << tokens.size();
        logwrite( function, message.str() );
        error = ERROR;
      }

      // Try converting it to a double, not because it's needed as a double but this checks
      // that it is indeed a double. stod will discard the "mm" after the double value so
      // there is no need to remove it prior.
      //
      if ( error==NO_ERROR ) {
        double focus = std::stod( tokens.at(1) );
        reply << std::fixed << std::setprecision(2) << focus;
      }
    }
    catch( std::out_of_range &e ) {
      logwrite( function, "EXCEPTION: out of range parsing focus from TCS reply string" );
      error = ERROR;
    }
    catch( std::invalid_argument &e ) {
      logwrite( function, "EXCEPTION converting focus to double" );
      error = ERROR;
    }

    retstring = reply.str();

    if ( !retstring.empty() && !silent ) logwrite( function, retstring );

    asyncmsg << "TCSD:getfocus:" << ( !retstring.empty() ? retstring : "ERROR" );
    this->async.enqueue( asyncmsg.str() );

    return error;
  }
  /***** TCS::Interface::get_focus ********************************************/


  /***** TCS::Interface::get_motion *******************************************/
  /**
   * @brief      get the motion status
   * @details    uses the ?MOTION command, returns a string instead of a code
   * @param[in]  arg        used only to request help
   * @param[out] retstring  contains the motion state string
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::get_motion( const std::string &arg, std::string &retstring ) {
    std::string function = "TCS::Interface::get_motion";
    std::stringstream message, asyncmsg;
    int motion_code = TCS_UNDEFINED;
    long error = NO_ERROR;

    // Help
    //
    if ( arg == "?" ) {
      retstring = TCSD_GET_MOTION;
      retstring.append( " \n" );
      retstring.append( "  Return the current motion state of the TCS\n" );
      return( NO_ERROR );
    }

    std::string motion;
    if ( this->send_command( "?MOTION", motion ) != NO_ERROR ) {
      logwrite( function, "ERROR getting motion state from TCS" );
      error = ERROR;
    }

    // Response will be a number that is converted to int here
    //
    try {
      if ( error == NO_ERROR ) motion_code = std::stoi( motion );
    }
    catch( std::out_of_range &e ) {
      message.str(""); message << "ERROR out of range parsing TCS reply \"" << motion << "\": " << e.what();
      logwrite( function, message.str() );
      motion_code = TCS_UNDEFINED;
    }
    catch( std::invalid_argument &e ) {
      message.str(""); message << "ERROR converting TCS reply \"" << motion << "\" to integer: " << e.what();
      logwrite( function, message.str() );
      motion_code = TCS_UNDEFINED;
    }

    // set the return string based on the TCS motion code
    //
    switch( motion_code ) {
      case TCS_MOTION_STOPPED:    retstring = TCS_MOTION_STOPPED_STR;
                                  break;
      case TCS_MOTION_SLEWING:    retstring = TCS_MOTION_SLEWING_STR;
                                  break;
      case TCS_MOTION_OFFSETTING: retstring = TCS_MOTION_OFFSETTING_STR;
                                  break;
      case TCS_MOTION_TRACKING:   retstring = TCS_MOTION_TRACKING_STR;
                                  break;
      case TCS_MOTION_SETTLING:   retstring = TCS_MOTION_SETTLING_STR;
                                  break;
      case TCS_MOTION_UNKNOWN:    retstring = TCS_MOTION_UNKNOWN_STR;
                                  break;
      default:                    retstring = TCS_MOTION_UNKNOWN_STR;
                                  message.str(""); message << "ERROR unrecognized motion code " << motion_code;
                                  logwrite( function, message.str() );
                                  error = ERROR;
                                  break;
    }

    asyncmsg << "TCSD:getmotion:" << ( !retstring.empty() ? retstring : "ERROR" );
    this->async.enqueue( asyncmsg.str() );

    return error;
  }
  /***** TCS::Interface::get_motion *******************************************/


  /***** TCS::Interface::ringgo ***********************************************/
  /**
   * @brief      wrapper for the native RINGGO command
   * @details    wraps the RINGGO command to ensure the value is within range
   * @param[in]  arg        requested angle string
   * @param[out] retstring  reference to return string for the command sent to the TCS
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::ringgo( const std::string &arg, std::string &retstring ) {
    std::string function = "TCS::Interface::ringgo";
    std::string retcode;
    std::stringstream message, asyncmsg;
    long error = ERROR;

    double angle = NAN;

    // Help
    //
    if ( arg == "?" ) {
      retstring = TCSD_RINGGO;
      retstring.append( " <angle>\n" );
      retstring.append( "  Rotate the cass ring to the specified angle in degrees.\n" );
      retstring.append( "  If the angle is available in more than one turn of the ring,\n" );
      retstring.append( "  the shortest move will be taken.\n" );
      return( NO_ERROR );
    }

    if ( arg.empty() ) { retstring="missing_argument"; return ERROR; }

    try {
      angle = std::stod( arg );
      error = NO_ERROR;
    }
    catch( std::out_of_range &e ) {
      message.str(""); message << "EXCEPTION: out of range parsing requested angle from \"" << arg << "\": " << e.what();
      logwrite( function, message.str() );
    }
    catch( std::invalid_argument &e ) {
      message.str(""); message << "EXCEPTION: converting requested angle from \"" << arg << "\" to double: " << e.what();
      logwrite( function, message.str() );
    }

    if ( std::isnan(angle) ) {
      logwrite( function, "ERROR: requested angle is NaN" );
      error = ERROR;
    }

    // For negative angles, offset them once by 360,
    //
    if ( angle < 0.0 ) angle += 360.0;

    // but if that isn't enough to make it positive then this is probably a bogus value.
    //
    if ( angle < 0.0 ) {
      message.str(""); message << "ERROR: requested angle " << angle << " is too negative";
      logwrite( function, message.str() );
      error = ERROR;
    }

    if ( angle > 359.99 && angle <= 360.0 ) angle = 0;

    if ( angle > 360.0 ) {
      message.str(""); message << "ERROR: requested angle " << angle << " cannot exceed 360";
      logwrite( function, message.str() );
      error = ERROR;
    }

#ifdef LOGLEVEL_DEBUG
    message.str(""); message << "[DEBUG] requested cass angle " << angle;
    logwrite( function, message.str() );
#endif

    std::stringstream cmd;
    cmd << "RINGGO " << std::fixed << std::setprecision(2) << angle;

    if ( error != ERROR ) error = this->send_command( cmd.str(), retcode );

    this->parse_reply_code( retcode, retstring );

    asyncmsg << "TCSD:ringgo:" << retstring;
    this->async.enqueue( asyncmsg.str() );

    return error;
  }
  /***** TCS::Interface::ringgo ***********************************************/


  /***** TCS::Interface::coords ***********************************************/
  /**
   * @brief      wrapper for the native COORDS command
   * @details    wraps the COORDS command to provide a friendly reply
   * @param[in]  args       requested angle string
   * @param[out] retstring  reference to return string for the command sent to the TCS
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::coords( std::string args, std::string &retstring ) {
    std::string function = "TCS::Interface::coords";
    std::string retcode;
    std::stringstream message, asyncmsg;

    // Help
    //
    if ( args == "?" ) {
      retstring = TCSD_COORDS;
      retstring.append( " <ra> <dec> <equinox> <ramotion> <decmotion> [<motionflag>] [ \"<targetname>\" ]\n" );
      retstring.append( "  Supply a position to the TCS.\n" );
      retstring.append( "  <ra> and <dec> are in decimal degrees\n" );
      retstring.append( "  <equinox> = 0 indicates apparent equinox\n" );
      retstring.append( "  Optional <motionflag> defines the meaning of <ramotion> and <decmotion> as\n" );
      retstring.append( "  0 (or absent): proper motion in units of RA 0.0001 sec/yr, DEC 0.001 arcsec/yr\n" );
      retstring.append( "  1: offset tracking rates in arcsec/hr\n" );
      retstring.append( "  2: offset tracking rates in sec/hr, arcsec/hr\n" );
      retstring.append( "  Optional targetname must be in double quotes, if supplied.\n" );
      return( NO_ERROR );
    }

    // check minimum/maximum number of arguments
    //
    std::vector<std::string> tokens;
    int ntok = Tokenize( args, tokens, " " );
    if ( ntok < 5 || ntok > 7 ) { retstring="invalid_arguments"; return ERROR; }

    std::stringstream cmd;
    cmd << "COORDS " << args;

    long error = this->send_command( cmd.str(), retcode );

    if ( error == NO_ERROR ) this->parse_reply_code( retcode, retstring );

    asyncmsg << "TCSD:coords:" << ( error == ERROR ? "ERROR" : retstring );
    this->async.enqueue( asyncmsg.str() );

    return error;
  }
  /***** TCS::Interface::coords ***********************************************/


  /***** TCS::Interface::send_command *****************************************/
  /**
   * @brief      writes the raw command, as received, to the TCS
   * @param[in]  cmd    string to send to TCS
   * @param[out] reply  reference to string to contain reply
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::send_command( std::string cmd, std::string &reply ) {
    std::string function = "TCS::Interface::send_command";
    std::stringstream message;
    std::string sbuf;

    if ( this->name.empty() ) {
      message.str(""); message << "ERROR sending \"" << cmd << "\": no connection open to the TCS" ;
      logwrite( function, message.str() );
      return( ERROR );
    }

    // Find the currently opened TCS (this->name) in the tcsmap.
    // It must be located and connected in order to proceed.
    //
    auto tcs_loc = this->tcsmap.find( this->name );

    // Is it configured?
    //
    if ( tcs_loc == tcsmap.end() ) {
      message.str(""); message << "ERROR sending \"" << cmd << "\": no TCS " << this->name << " not configured";
      logwrite( function, message.str() );
      return( ERROR );
    }

    // Is it connected?
    //
    if ( ! tcs_loc->second.isconnected() ) {
      message.str(""); message << "ERROR sending \"" << cmd << "\": no connection open to TCS " << this->name;
      logwrite( function, message.str() );
      tcs_loc->second.close();
      return( ERROR );
    }

    // TCS is good, send the command, read the reply
    //
    if ( tcs_loc->second.send( cmd, reply ) != NO_ERROR ) {
      message.str(""); message << "ERROR writing \"" << cmd << "\" to TCS on fd " << tcs_loc->second.fd();
      logwrite( function, message.str() );
      return( ERROR );
    }

    return( NO_ERROR );
  }
  /***** TCS::Interface::send_command *****************************************/


  /***** TCS::Interface::parse_reply_code *************************************/
  /**
   * @brief      parse the TCS reply code into a friendly string
   * @param[in]  codein  input reply code string
   * @param[out] reply   reference to string to contain friendly reply
   *
   */
  void Interface::parse_reply_code( std::string codein, std::string &reply ) {
    std::string function = "TCS::Interface::parse_reply_code";
    std::stringstream message;
    int code = TCS_UNDEFINED;

    // Convert the reply code from string to integer
    //
    try {
      code = std::stoi( codein );
    }
    catch( std::out_of_range &e ) {
      message.str(""); message << "EXCEPTION: out of range parsing TCS return value \"" << codein << "\": " << e.what();
      logwrite( function, message.str() );
    }
    catch( std::invalid_argument &e ) {
      message.str(""); message << "EXCEPTION: converting TCS return value \"" << codein << "\" to integer: " << e.what();
      logwrite( function, message.str() );
    }

    // Convert the reply code integer to a friendly string
    //
    switch ( code ) {
      case TCS_SUCCESS:              reply = TCS_SUCCESS_STR;
                                     break;
      case TCS_UNRECOGNIZED_COMMAND: reply = TCS_UNRECOGNIZED_COMMAND;
                                     break;
      case TCS_INVALID_PARAMETER:    reply = TCS_INVALID_PARAMETER;
                                     break;
      case TCS_UNABLE_TO_EXECUTE:    reply = TCS_UNABLE_TO_EXECUTE;
                                     break;
      case TCS_HOST_UNAVAILABLE:     reply = TCS_HOST_UNAVAILABLE;
                                     break;
      case TCS_UNDEFINED:
      default:                       reply = TCS_UNDEFINED_STR;
                                     break;
    }
    return;
  }
  /***** TCS::Interface::parse_reply_code *************************************/

}
