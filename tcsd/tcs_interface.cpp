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


  /***** TCS::Interface::initialize_class *************************************/
  /**
   * @brief      initialize class variables
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::initialize_class() {
    std::string function = "TCS::Interface::initialize_class";
    std::stringstream message;

    if ( this->port < 0 || host.empty() ) {
      message.str(""); message << "ERROR: host \"" << this->host << "\" and/or port \"" << this->port << "\" invalid";
      logwrite( function, message.str() );
      return( ERROR );
    }

    Network::TcpSocket s( this->host, this->port );
    this->tcs = s;

    logwrite( function, "interface initialized ok" );
    return( NO_ERROR );
  }
  /***** TCS::Interface::initialize_class *************************************/


  /***** TCS::Interface::open *************************************************/
  /**
   * @brief      opens the TCS socket connection
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::open( ) {
    std::string function = "TCS::Interface::open";
    std::stringstream message;

    if ( this->tcs.isconnected() ) {
      logwrite( function, "connection already open" );
      return( NO_ERROR );
    }

    // initialize connection to the TCS
    //
    logwrite( function, "opening connection to TCS" );

    if ( this->tcs.Connect() != 0 ) {
      message.str(""); message << "ERROR connecting to TCS on " << this->host << ":" << this->port << " ...";
      logwrite( function, message.str() );
      return( ERROR );
    }

    message.str(""); message << "socket connection to "
                             << this->tcs.gethost() << ":" << this->tcs.getport()
                             << " established on fd " << this->tcs.getfd();
    logwrite( function, message.str() );

    return( NO_ERROR );
  }
  /***** TCS::Interface::open *************************************************/


  /***** TCS::Interface::close ************************************************/
  /**
   * @brief      closes the TCS socket connection
   * @return     ERROR or NO_ERROR
   *
   */
  long Interface::close( ) {
    std::string function = "TCS::Interface::close";
    std::stringstream message;

    if ( !this->tcs.isconnected() ) {
      logwrite( function, "connection already closed" );
      return( NO_ERROR );
    }

    long error = this->tcs.Close();

    if ( error == NO_ERROR ) {
      logwrite( function, "connection to TCS closed" );
    }
    else {
      logwrite( function, "ERROR closing connection to TCS" );
    }
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
    std::stringstream message;
    std::string weather;

    // Send the WEATHER command to the TCS. This returns a string of key=val pairs
    // with each pair separated by a newline character. The first two pairs are 
    // RA and DEC, which is all that is needed here.
    //
    if ( this->send_command( "?WEATHER", weather ) != NO_ERROR ) {
      logwrite( function, "ERROR getting coords from TCS" );
      return ERROR;
    }

    std::vector<std::string> pairs;
    Tokenize( weather, pairs, "\n" );  // tokenize on the newline to break into key=val pairs

    // If someone ever changes the TCS message then this will have to be changed.
    //
    if ( pairs.size() != 14 ) {
      logwrite( function, "ERROR malformed reply" );
      return ERROR;
    }

    // Tokenize the first two key=val pairs on the "=" to get the ra, dec values
    //
    try {
      std::vector<std::string> ra_tokens, dec_tokens;

      Tokenize( pairs.at(0), ra_tokens, "=" );
      Tokenize( pairs.at(1), dec_tokens, "=" );

      message.str(""); message << ra_tokens.at(1) << " " << dec_tokens.at(1);
    }
    catch( std::out_of_range &e ) {
      logwrite( function, "ERROR out of range parsing ra,dec from ?WEATHER command" );
      return ERROR;
    }

    logwrite( function, message.str() );

    retstring = message.str();

    return NO_ERROR;
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
    std::stringstream message;

    // Send the REQPOS command to the TCS. This returns a string that looks like:
    //
    // tcsreply = "UTC = ddd hh:mm:ss, LST = hh:mm:ss\n
    //             RA = hh:mm:ss.ss, DEC = dd:mm:ss.s, HA=hh:mm:ss.s\n
    //             air mass = aa.aaa"
    //
    std::string tcsreply;
    if ( this->send_command( "REQPOS", tcsreply ) != NO_ERROR ) {
      logwrite( function, "ERROR getting coords from TCS" );
      return ERROR;
    }

    // First tokenize on newline
    //
    std::vector<std::string> lines;
    Tokenize( tcsreply, lines, "\n" );

    if ( lines.size() != 3 ) {
      message.str(""); message << "ERROR expected 3 lines from REQPOS string but received " << lines.size();
      logwrite( function, message.str() );
      return( ERROR );
    }

    // Then the 2nd token, lines[1], contains the RA,DEC so try to
    // tokenize that on the comma.
    //
    try {
      std::vector<std::string> radec;
      Tokenize( lines.at(1), radec, "," );  // lines[1] = "RA = hh:mm:ss.ss, DEC = dd:mm:ss.s, HA=hh:mm:ss.s\n"

      if ( radec.size() != 3 ) {
        message.str(""); message << "ERROR expected 3 tokens for ra, dec, ha but received " << radec.size();
        logwrite( function, message.str() );
        return( ERROR );
      }

      // Now try to tokenize the first two tokens of line1 on the equal sign to extract RA and DEC values
      //
      std::vector<std::string> ra, dec;
      Tokenize( radec.at(0), ra,  "=" );
      Tokenize( radec.at(1), dec, "=" );

      if ( ra.size() != 2 && dec.size() != 2 ) {
        message.str(""); message << "ERROR extracting ra,dec from " << lines.at(1);
        logwrite( function, message.str() );
        return( ERROR );
      }

      message.str(""); message << ra.at(1) << " " << dec.at(1);
    }
    catch( std::out_of_range &e ) {
      logwrite( function, "EXCEPTION: out of range parsing ra,dec from TCS reply string" );
      return ERROR;
    }

    logwrite( function, message.str() );

    retstring = message.str();

    return NO_ERROR;
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
    std::stringstream message;
    std::stringstream reply;

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
      return ERROR;
    }

    // First tokenize on comma
    //
    std::vector<std::string> lines;
    Tokenize( tcsreply, lines, "\n" );

    if ( lines.size() != 5 ) {
      message.str(""); message << "ERROR expected 5 lines from REQSTAT string but received " << lines.size()
                               << " in reply \"" << tcsreply << "\"";
      logwrite( function, message.str() );
      return( ERROR );
    }

    // Then the 5th token, lines[4], contains the "Cass ring angle = dd.dd"
    // tokenize that on the equal to get the value.
    //
    try {
      std::vector<std::string> tokens;
      Tokenize( lines.at(4), tokens, "=" );  // lines[4] = "Cass ring angle = dd.dd"

      if ( tokens.size() != 2 ) {
        message.str(""); message << "ERROR expected 2 tokens for Cass ring angle = dd.dd but received " << tokens.size();
        logwrite( function, message.str() );
        return( ERROR );
      }

      // Try converting it to a double, not because it's needed as a double but this checks
      // that it is indeed a double
      //
      double angle = std::stod( tokens.at(1) );

      reply << std::fixed << std::setprecision(2) << angle;
    }
    catch( std::out_of_range &e ) {
      logwrite( function, "EXCEPTION: out of range parsing Cass ring angle from TCS reply string" );
      return ERROR;
    }
    catch( std::invalid_argument &e ) {
      logwrite( function, "EXCEPTION converting Cass ring angle to double" );
      return ERROR;
    }

    retstring = reply.str();

    logwrite( function, retstring );

    return NO_ERROR;
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
    std::stringstream message;
    std::string weather;

    // Send the WEATHER command to the TCS. This returns a string of key=val pairs
    // with each pair separated by a newline character. The first two pairs are 
    // RA and DEC, which is all that is needed here.
    //
    if ( this->send_command( "?WEATHER", weather ) != NO_ERROR ) {
      logwrite( function, "ERROR getting coords from TCS" );
      return ERROR;
    }

    std::vector<std::string> pairs;
    Tokenize( weather, pairs, "\n" );  // tokenize on the newline to break into key=val pairs

    // If someone ever changes the TCS message then this will have to be changed.
    //
    if ( pairs.size() != 14 ) {
      logwrite( function, "ERROR malformed reply" );
      return ERROR;
    }

    // Tokenize the first two key=val pairs on the "=" to get the ra, dec values
    //
    try {
      std::vector<std::string> telazi_tokens, domeazi_tokens;

      Tokenize( pairs.at(5), telazi_tokens, "=" );
      Tokenize( pairs.at(8), domeazi_tokens, "=" );

      message.str(""); message << domeazi_tokens.at(1) << " " << telazi_tokens.at(1);
    }
    catch( std::out_of_range &e ) {
      logwrite( function, "ERROR out of range parsing azimuth from ?WEATHER command" );
      return ERROR;
    }

    retstring = message.str();

    return NO_ERROR;
  }
  /***** TCS::Interface::get_dome *********************************************/


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
    std::stringstream message;
    int motion_code = TCS_UNDEFINED;
    long error = NO_ERROR;

    std::string motion;
    if ( this->send_command( "?MOTION", motion ) != NO_ERROR ) {
      logwrite( function, "ERROR getting motion state from TCS" );
      return ERROR;
    }

    // Response will be a number that is converted to int here
    //
    try {
      motion_code = std::stoi( motion );
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
    std::stringstream message;

    double angle = NAN;

    try {
      angle = std::stod( args );
    }
    catch( std::out_of_range &e ) {
      message.str(""); message << "EXCEPTION: out of range parsing requested angle from \"" << args << "\": " << e.what();
      logwrite( function, message.str() );
      return ERROR;
    }
    catch( std::invalid_argument &e ) {
      message.str(""); message << "EXCEPTION: converting requested angle from \"" << args << "\" to double: " << e.what();
      logwrite( function, message.str() );
      return ERROR;
    }

    if ( std::isnan(angle) ) {
      logwrite( function, "ERROR: requested angle is NaN" );
      return ERROR;
    }

    if ( angle < 0.0 ) angle += 360.0;

    if ( angle > 359.99 && angle <= 360.0 ) angle = 0;

    if ( angle > 360.0 ) {
      message.str(""); message << "ERROR: requested angle " << angle << " cannot exceed 360";
      logwrite( function, message.str() );
      return ERROR;
    }

#ifdef LOGLEVEL_DEBUG
    message.str(""); message << "[DEBUG] requested cass angle " << angle;
    logwrite( function, message.str() );
#endif

    std::stringstream cmd;
    cmd << "RINGGO " << std::fixed << std::setprecision(2) << angle;

    return this->send_command( cmd.str(), retstring );
  }
  /***** TCS::Interface::ringgo ***********************************************/


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
      logwrite( function, "ERROR: no connection open to the TCS" );
      return( ERROR );
    }

#ifdef LOGLEVEL_DEBUG  // this can be a little much when polling
//  message.str(""); message << "[DEBUG] sending to TCS on fd " << this->tcs.getfd() << ": " << cmd;
//  logwrite( function, message.str() );
#endif

    cmd.append( term );        // add terminator before sending command to the TCS

    if ( this->tcs.Write( cmd ) == -1 ) {
      message.str(""); message << "ERROR writing to TCS on fd " << this->tcs.getfd();
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
                                                 << " waiting for response from TCS"; error = TIMEOUT; }
        if ( ret <0 ) { message.str(""); message << "ERROR polling fd "   << tcs.getfd()
                                                 << " waiting for response from TCS"; error = ERROR; }
        if ( error != NO_ERROR ) logwrite( function, message.str() );
        break;
      }

      // data available, read from socket
      //
      if ( ( ret = this->tcs.Read( sbuf ) ) <= 0 ) {   // all TCS respoonses are terminated with NUL
        if ( ret < 0 ) {
          message.str(""); message << "ERROR reading from TCS on fd " << tcs.getfd() << ": " << strerror(errno);
          logwrite( function, message.str() );
        }
        if ( ret == 0 ) {
          message.str(""); message << "TIMEOUT reading from TCS on fd " << tcs.getfd() << ": " << strerror(errno);
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

}
