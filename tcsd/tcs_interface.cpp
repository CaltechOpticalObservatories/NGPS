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

  /***** TCS::Interface::Interface ********************************************/
  /**
   * @brief      class constructor
   *
   */
  Interface::Interface() {
  }
  /***** TCS::Interface::Interface ********************************************/


  /***** TCS::Interface::~Interface *******************************************/
  /**
   * @brief      class deconstructor
   *
   */
  Interface::~Interface() {
  }
  /***** TCS::Interface::~Interface *******************************************/


  /***** TCS::Interface::list *************************************************/
  /**
   * @brief      list configured TCS devices
   * @param[out] retstring  reference to string to contain the list of devices
   *
   */
  void Interface::list( std::string &retstring ) {
    std::string function = "TCS::Interface::list";
    std::stringstream message;

    message << "name host:port\n";

    for ( auto it = this->tcsmap.begin(); it != this->tcsmap.end(); ++it ) {
      message << it->first << " " << it->second.host << ":" << it->second.port << "\n";
    }

    retstring = message.str();

    return;
  }
  /***** TCS::Interface::list *************************************************/


  /***** TCS::Interface::llist ************************************************/
  /**
   * @brief      line list configured TCS devices
   * @details    same as list except all results come out on a single line
   * @param[out] retstring  reference to string to contain the list of devices
   *
   */
  void Interface::llist( std::string &retstring ) {
    std::string function = "TCS::Interface::llist";
    std::stringstream message, asyncmsg;

    int count=0;
    for ( auto it = this->tcsmap.begin(); it != this->tcsmap.end(); ++it, ++count ) {
      if ( count > 0 ) message << ",";  // add a comma separator if coming through here again
      message << it->first << " " << it->second.host << ":" << it->second.port;
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
   * @param[in]  args  contains what to open, real or sim
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::open( std::string args, std::string &retstring ) {
    std::string function = "TCS::Interface::open";
    std::stringstream message, asyncmsg;
    long error = NO_ERROR;

    // Typically, a second call to open might not return an error but this function returns an error
    // on a second call to open because the user might be trying to open a different TCS, e.g.
    // opening the real while the sim is already open, so returning an error will catch that.
    //
    if ( error==NO_ERROR && this->tcs.isconnected() ) {
      message.str(""); message << "ERROR:connection already open to " << this->tcs.gethost() << ":" << this->tcs.getport();
      logwrite( function, message.str() );
      error = ERROR;
    }

    // Need the name of the tcs to connect to
    //
    if ( error==NO_ERROR && args.empty() ) {
      message.str(""); message << "ERROR:must specify a valid TCS name";
      logwrite( function, message.str() );
      error = ERROR;
    }

    // Find the requested name in the tcsmap
    //
    auto tcsloc = this->tcsmap.find( args );

    if ( error==NO_ERROR && tcsloc == this->tcsmap.end() ) {
      message.str(""); message << "ERROR:requested TCS name \"" << args << "\" not found in configured list";
      logwrite( function, message.str() );
      error = ERROR;
    }

    if ( error == NO_ERROR ) {
      // Configure the tcs class
      //
      this->tcs.sethost( tcsloc->second.host );
      this->tcs.setport( tcsloc->second.port );

      // initialize connection to the TCS
      //
      message.str(""); message << "opening connection to TCS " << tcsloc->first << " on " << this->tcs.gethost() << ":" << this->tcs.getport();
      logwrite( function, message.str() );

      if ( this->tcs.Connect() != 0 ) {
        message.str(""); message << "ERROR connecting to TCS " << tcsloc->first << " on " << this->tcs.gethost() << ":" << this->tcs.getport();
        logwrite( function, message.str() );
        this->name="";
        error = ERROR;
      }

      // Save the name of this tcs to the class upon success
      //
      this->name = tcsloc->first;

      message.str(""); message << "connected to " << this->name << " "
                               << this->tcs.gethost() << ":" << this->tcs.getport()
                               << " on fd " << this->tcs.getfd();
      logwrite( function, message.str() );
      error = this->isopen( retstring );
    }

    asyncmsg << "TCSD:open:" << ( !retstring.empty() ? retstring : "ERROR" );
    this->async.enqueue( asyncmsg.str() );

    return( error );
  }
  /***** TCS::Interface::open *************************************************/


  /***** TCS::Interface::isopen ***********************************************/
  /**
   * @brief      return the open status of the TCS
   * @param[out] retstring  contains the open status
   * @return     ERROR or NO_ERROR
   *
   * The retstring contains either the name of the connected TCS device (if open)
   * or false (if closed).
   *
   */
  long Interface::isopen( std::string &retstring ) {
    std::string function = "TCS::Interface::isopen";
    std::stringstream message, asyncmsg;
    long error = NO_ERROR;

    // If connected then set the retstring to the name of the TCS to which we're connected
    //
    if ( this->tcs.isconnected() ) {
      if ( this->name.empty() ) {
        retstring="ERROR";
        message.str(""); message << "ERROR:connection open to un-named TCS. Try closing, reopening.";
        error = ERROR;
      }
      else {
        retstring=this->name;
        message.str(""); message << "connection open to " << this->name;
      }
      logwrite( function, message.str() );
    }
    else retstring = "false";  // if not connected then retstring is set to "false"

    asyncmsg << "TCSD:isopen:" << retstring;
    this->async.enqueue( asyncmsg.str() );
    return(error);
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

    if ( !this->tcs.isconnected() ) {
      logwrite( function, "connection already closed" );
    }
    else {
      error = this->tcs.Close();

      if ( error == NO_ERROR ) {
        logwrite( function, "connection to TCS closed" );
      }
      else {
        logwrite( function, "ERROR closing connection to TCS" );
      }
    }

    asyncmsg << "TCSD:close:" << ( error==NO_ERROR ? "DONE" : "ERROR" );
    this->async.enqueue( asyncmsg.str() );

    return( error );
  }
  /***** TCS::Interface::close ************************************************/


  /***** TCS::Interface::get_weather_coords ***********************************/
  /**
   * @brief      get the current simulator coords
   * @details    uses the ?WEATHER command, pulls out just the RA and DEC
   * @param[out] retstring  contains space-delimited ra dec
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::get_weather_coords( std::string &retstring ) {
    std::string function = "TCS::Interface::get_weather_coords";
    std::stringstream message, asyncmsg;
    std::string weather;
    long error = NO_ERROR;

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
   * @param[out] retstring  contains space-delimited ra dec
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::get_coords( std::string &retstring ) {
    std::string function = "TCS::Interface::get_coords";
    std::stringstream message, asyncmsg;
    long error = NO_ERROR;

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
   * @param[out] retstring  contains cass ring angle
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::get_cass( std::string &retstring ) {
    std::string function = "TCS::Interface::get_cass";
    std::stringstream message, asyncmsg;
    std::stringstream reply;
    long error = NO_ERROR;

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
   * @param[out] retstring  contains space-delimited ra dec
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::get_dome( std::string &retstring ) {
    std::string function = "TCS::Interface::get_dome";
    std::stringstream message, asyncmsg;
    std::string weather;
    long error = NO_ERROR;

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


  /***** TCS::Interface::get_focus ********************************************/
  /**
   * @brief      get the current focus position
   * @details    uses the REQSTAT command, pulls out just the focus,
   *             returns as "dd.dd"
   * @param[out] retstring  contains focus position
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::get_focus( std::string &retstring ) {
    std::string function = "TCS::Interface::get_focus";
    std::stringstream message, asyncmsg;
    std::stringstream reply;
    long error = NO_ERROR;

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

    if ( !retstring.empty() ) logwrite( function, retstring );

    asyncmsg << "TCSD:getfocus:" << ( !retstring.empty() ? retstring : "ERROR" );
    this->async.enqueue( asyncmsg.str() );

    return error;
  }
  /***** TCS::Interface::get_focus ********************************************/


  /***** TCS::Interface::get_motion *******************************************/
  /**
   * @brief      get the motion status
   * @details    uses the ?MOTION command, returns a string instead of a code
   * @param[out] retstring  contains the motion state string
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::get_motion( std::string &retstring ) {
    std::string function = "TCS::Interface::get_motion";
    std::stringstream message, asyncmsg;
    int motion_code = TCS_UNDEFINED;
    long error = NO_ERROR;

    std::string motion;
    if ( this->send_command( "?MOTION", motion ) != NO_ERROR ) {
      logwrite( function, "ERROR getting motion state from TCS" );
      error = ERROR;
    }

    // Response will be a number that is converted to int here
    //
    try {
      if ( error != ERROR ) motion_code = std::stoi( motion );
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
   * @param[in]  args       requested angle string
   * @param[out] retstring  reference to return string for the command sent to the TCS
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::ringgo( std::string args, std::string &retstring ) {
    std::string function = "TCS::Interface::ringgo";
    std::string retcode;
    std::stringstream message, asyncmsg;
    long error = ERROR;

    double angle = NAN;

    try {
      angle = std::stod( args );
      error = NO_ERROR;
    }
    catch( std::out_of_range &e ) {
      message.str(""); message << "EXCEPTION: out of range parsing requested angle from \"" << args << "\": " << e.what();
      logwrite( function, message.str() );
    }
    catch( std::invalid_argument &e ) {
      message.str(""); message << "EXCEPTION: converting requested angle from \"" << args << "\" to double: " << e.what();
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
    long error=NO_ERROR;
    int ret;
    std::string sbuf;
    std::string term = "\r";   /// all commands to TCS are terminated with this

    if ( !this->tcs.isconnected() ) {
      message.str(""); message << "ERROR sending \"" << cmd << "\": no connection open to the TCS" ;
      logwrite( function, message.str() );
      return( ERROR );
    }

#ifdef LOGLEVEL_DEBUG  // this can be a little much when polling
//  message.str(""); message << "[DEBUG] sending to TCS on fd " << this->tcs.getfd() << ": " << cmd;
//  logwrite( function, message.str() );
#endif

    cmd.append( term );        // add terminator before sending command to the TCS

    if ( this->tcs.Write( cmd ) == -1 ) {
      message.str(""); message << "ERROR writing \"" << cmd << "\" to TCS on fd " << this->tcs.getfd();
      logwrite( function, message.str() );
      return( ERROR );
    }

    // receive the reply
    //
    // wait for incoming data
    //
    do {
      // wait for incoming data
      //
      if ( ( ret = this->tcs.Poll() ) <= 0 ) {
        if ( ret==0 ) { message.str(""); message << "TIMEOUT polling fd " << tcs.getfd()
                                                 << " waiting for TCS response to \"" << cmd << "\""; error = TIMEOUT; }
        if ( ret <0 ) { message.str(""); message << "ERROR polling fd "   << tcs.getfd()
                                                 << " waiting for TCS response to \"" << cmd << "\""; error = ERROR; }
        if ( error != NO_ERROR ) logwrite( function, message.str() );
        break;
      }

      // data available, read from socket
      //
      if ( ( ret = this->tcs.Read( sbuf ) ) <= 0 ) {   // all TCS respoonses are terminated with NUL
        if ( ret < 0 ) {
          message.str(""); message << "ERROR reading TCS response to \"" << cmd << "\" on fd " << tcs.getfd() << ": " << strerror(errno);
          logwrite( function, message.str() );
        }
        if ( ret == 0 ) {
          message.str(""); message << "TIMEOUT reading TCS response to \"" << cmd << "\" on fd " << tcs.getfd() << ": " << strerror(errno);
          logwrite( function, message.str() );
        }
        break;
      }
    } while ( ret > 0 && sbuf.find( "\0" ) == std::string::npos );


    // remove any trailing linefeed and carriage return
    //
    sbuf.erase(std::remove(sbuf.begin(), sbuf.end(), '\r' ), sbuf.end());

    reply = sbuf;

#ifdef LOGLEVEL_DEBUG  // this can be a little much when polling
//  message.str(""); message << "[DEBUG] received from TCS on fd " << this->tcs.getfd() << ": " << reply;
//  logwrite( function, message.str() );
#endif

    return( error );
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
